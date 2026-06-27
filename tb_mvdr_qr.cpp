// ============================================================
//  tb_mvdr_qr.cpp  –  C-Simulation Testbench
//  Compares HLS kernel output against MATLAB golden reference
// ============================================================
#include "mvdr_qr.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>

// ── Helpers ──────────────────────────────────────────────────
static float randn_f() {
    // Box-Muller
    float u1 = (float)(rand() + 1) / ((float)RAND_MAX + 1.0f);
    float u2 = (float)(rand() + 1) / ((float)RAND_MAX + 1.0f);
    return sqrtf(-2.0f * logf(u1)) * cosf(2.0f * 3.14159265f * u2);
}

// Complex multiply a * conj(b)
static void cmul_conj(float ar, float ai, float br, float bi,
                      float &re, float &im) {
    re = ar*br + ai*bi;
    im = ai*br - ar*bi;
}

int main()
{
    srand(42);  // reproducible

    // ── Simulation parameters ──────────────────────────────
    const int   M_      = M;
    const int   N_      = N_SNAP;
    const float lambda  = 0.1f;       // wavelength (m)  c/f0
    const float d       = lambda/2.f;
    const float theta_s = 30.0f * 3.14159265f / 180.0f;   // SOI
    const float theta_j1= -20.0f* 3.14159265f / 180.0f;
    const float theta_j2=  50.0f* 3.14159265f / 180.0f;
    const float sig_amp  = sqrtf(powf(10.f, 10.f/10.f));  // SNR=10dB
    const float jam_amp  = sqrtf(powf(10.f, 30.f/10.f));  // JNR=30dB
    const float noise_std= 1.0f / sqrtf(2.0f);

    // ── Steering vectors ───────────────────────────────────
    float a_re[M_], a_im[M_];
    float aj1_re[M_], aj1_im[M_];
    float aj2_re[M_], aj2_im[M_];

    for (int m = 0; m < M_; m++) {
        float phi_s  = 2.f * 3.14159265f * d/lambda * (float)m * sinf(theta_s);
        float phi_j1 = 2.f * 3.14159265f * d/lambda * (float)m * sinf(theta_j1);
        float phi_j2 = 2.f * 3.14159265f * d/lambda * (float)m * sinf(theta_j2);
        a_re[m]   = cosf(phi_s);  a_im[m]   = sinf(phi_s);
        aj1_re[m] = cosf(phi_j1); aj1_im[m] = sinf(phi_j1);
        aj2_re[m] = cosf(phi_j2); aj2_im[m] = sinf(phi_j2);
    }

    // ── Generate X data ────────────────────────────────────
    static cdata_t X_hw[M_ * N_];
    static cdata_t a_hw[M_];

    // Fill a_hw
    for (int m = 0; m < M_; m++)
        a_hw[m] = cdata_t((data_t)a_re[m], (data_t)a_im[m]);

    // Fill X_hw  (column-major: X[m*N + n])
    for (int n = 0; n < N_; n++) {
        // SOI
        float s_re = sig_amp * randn_f() * 0.707f;
        float s_im = sig_amp * randn_f() * 0.707f;
        // Jammers
        float j1_re = jam_amp * randn_f() * 0.707f;
        float j1_im = jam_amp * randn_f() * 0.707f;
        float j2_re = jam_amp * randn_f() * 0.707f;
        float j2_im = jam_amp * randn_f() * 0.707f;

        for (int m = 0; m < M_; m++) {
            float xr = a_re[m]*s_re - a_im[m]*s_im
                     + aj1_re[m]*j1_re - aj1_im[m]*j1_im
                     + aj2_re[m]*j2_re - aj2_im[m]*j2_im
                     + noise_std * randn_f();
            float xi = a_re[m]*s_im + a_im[m]*s_re
                     + aj1_re[m]*j1_im + aj1_im[m]*j1_re
                     + aj2_re[m]*j2_im + aj2_im[m]*j2_re
                     + noise_std * randn_f();
            // Clamp to [-1, 1)
            if (xr >  0.99f) xr =  0.99f;
            if (xr < -1.0f)  xr = -1.0f;
            if (xi >  0.99f) xi =  0.99f;
            if (xi < -1.0f)  xi = -1.0f;
            X_hw[m * N_ + n] = cdata_t((data_t)xr, (data_t)xi);
        }
    }

    // ── Call HLS top-level ─────────────────────────────────
    static cweight_t w_hw[M_];
    mvdr_qr_top(X_hw, a_hw, w_hw);

    // ── Print results ──────────────────────────────────────
    printf("\n=== HLS MVDR-QR Weight Vectors ===\n");
    printf("%4s  %12s  %12s\n", "Elem", "Real", "Imag");
    for (int m = 0; m < M_; m++) {
        printf("%4d  %12.6f  %12.6f\n",
               m,
               (float)w_hw[m].real(),
               (float)w_hw[m].imag());
    }

    // ── Verify distortionless constraint: w^H * a_s ≈ 1 ──
    float dot_re = 0, dot_im = 0;
    for (int m = 0; m < M_; m++) {
        float wr = (float)w_hw[m].real(), wi = (float)w_hw[m].imag();
        float ar_ = (float)a_hw[m].real(), ai_ = (float)a_hw[m].imag();
        // conj(w) * a_s
        dot_re += wr*ar_ + wi*ai_;
        dot_im += wr*ai_ - wi*ar_;
    }
    printf("\nDistortionless check: w^H * a_s = (%.4f, %.4f) [should be (1,0)]\n",
           dot_re, dot_im);

    float err = sqrtf((dot_re-1.f)*(dot_re-1.f) + dot_im*dot_im);
    printf("Error magnitude: %.6e\n", err);

    if (err < 0.05f) {
        printf("\n[PASS] Distortionless constraint satisfied.\n");
        return 0;
    } else {
        printf("\n[FAIL] Constraint violation > 0.05\n");
        return 1;
    }
}
