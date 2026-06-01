// ============================================================
//  host_mvdr_qr.cpp  –  XRT Host Application
//  Versal VCK190 | Vitis 2024.1
//  Build: g++ -O2 host_mvdr_qr.cpp -o host -lxrt_coreutil
//              -I${XILINX_XRT}/include -L${XILINX_XRT}/lib
// ============================================================
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_bo.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <complex>
#include <vector>
#include <chrono>
#include <cassert>
#include <string>

// ─── Fixed-point type sizes ──────────────────────────────────
// data_t / weight_t = ap_fixed<16,2> stored as int16_t (2 bytes)
// Complex: {re, im} packed as two int16_t
using fp16_t = int16_t;

struct cfp16 { fp16_t re, im; };

static const int M_     = 8;
static const int N_SNAP_= 64;
static const float SCALE_IN  = (float)(1 << 14);  // Q1.14
static const float SCALE_OUT = (float)(1 << 14);

// ─── Float helpers ───────────────────────────────────────────
static float pi = 3.14159265358979f;

static float randn()
{
    float u1 = (float)(rand()+1)/((float)RAND_MAX+1.f);
    float u2 = (float)(rand()+1)/((float)RAND_MAX+1.f);
    return sqrtf(-2.f*logf(u1))*cosf(2.f*pi*u2);
}

static int16_t to_fp16(float x)
{
    int v = (int)roundf(x * SCALE_IN);
    if (v >  32767) v =  32767;
    if (v < -32768) v = -32768;
    return (int16_t)v;
}

static float from_fp16(int16_t x)
{
    return (float)x / SCALE_OUT;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Usage: %s <xclbin>\n", argv[0]);
        return 1;
    }
    std::string xclbin_path = argv[1];

    printf("=== QR-MVDR Beamformer on Versal ===\n");
    printf("M=%d  N_SNAP=%d\n\n", M_, N_SNAP_);

    // ── 1. Open Device & Load xclbin ────────────────────────
    auto device = xrt::device(0);
    auto uuid   = device.load_xclbin(xclbin_path);
    auto kernel = xrt::kernel(device, uuid, "mvdr_qr_top");

    printf("[XRT] xclbin loaded. Device: %s\n",
           device.get_info<xrt::info::device::name>().c_str());

    // ── 2. Allocate Buffers ──────────────────────────────────
    size_t sz_X   = M_ * N_SNAP_ * sizeof(cfp16);
    size_t sz_a   = M_ * sizeof(cfp16);
    size_t sz_w   = M_ * sizeof(cfp16);

    auto bo_X   = xrt::bo(device, sz_X,  kernel.group_id(0));
    auto bo_as  = xrt::bo(device, sz_a,  kernel.group_id(1));
    auto bo_w   = xrt::bo(device, sz_w,  kernel.group_id(2));

    // ── 3. Generate Test Data (float -> fixed-point) ─────────
    // Steering vector for SOI at 30 degrees
    std::vector<std::complex<float>> a_s(M_);
    float lambda = 0.1f, d = lambda/2.f;
    float theta_s = 30.f * pi / 180.f;
    for (int m = 0; m < M_; m++) {
        float phi = 2.f*pi*d/lambda*(float)m*sinf(theta_s);
        a_s[m] = std::complex<float>(cosf(phi), sinf(phi));
    }

    // Jammer steering vectors
    float theta_j1 = -20.f*pi/180.f, theta_j2 = 50.f*pi/180.f;
    std::vector<std::complex<float>> aj1(M_), aj2(M_);
    for (int m = 0; m < M_; m++) {
        float p1 = 2.f*pi*d/lambda*(float)m*sinf(theta_j1);
        float p2 = 2.f*pi*d/lambda*(float)m*sinf(theta_j2);
        aj1[m] = {cosf(p1), sinf(p1)};
        aj2[m] = {cosf(p2), sinf(p2)};
    }

    srand(42);
    float sig_amp = sqrtf(powf(10.f,10.f/10.f));
    float jam_amp = sqrtf(powf(10.f,30.f/10.f));

    cfp16* X_ptr = bo_X.map<cfp16*>();
    cfp16* a_ptr = bo_as.map<cfp16*>();

    // Fill X (column-major: X[m*N + n])
    for (int n = 0; n < N_SNAP_; n++) {
        float s_re = sig_amp * randn() * 0.707f;
        float s_im = sig_amp * randn() * 0.707f;
        float j1r  = jam_amp * randn() * 0.707f;
        float j1i  = jam_amp * randn() * 0.707f;
        float j2r  = jam_amp * randn() * 0.707f;
        float j2i  = jam_amp * randn() * 0.707f;
        for (int m = 0; m < M_; m++) {
            float xr = a_s[m].real()*s_re  - a_s[m].imag()*s_im
                     + aj1[m].real()*j1r   - aj1[m].imag()*j1i
                     + aj2[m].real()*j2r   - aj2[m].imag()*j2i
                     + randn()*0.707f;
            float xi = a_s[m].real()*s_im  + a_s[m].imag()*s_re
                     + aj1[m].real()*j1i   + aj1[m].imag()*j1r
                     + aj2[m].real()*j2i   + aj2[m].imag()*j2r
                     + randn()*0.707f;
            // Normalize to fit Q1.14
            xr = std::max(-0.99f, std::min(0.99f, xr * 0.03f));
            xi = std::max(-0.99f, std::min(0.99f, xi * 0.03f));
            X_ptr[m * N_SNAP_ + n] = {to_fp16(xr), to_fp16(xi)};
        }
    }

    // Fill a_s
    for (int m = 0; m < M_; m++)
        a_ptr[m] = {to_fp16(a_s[m].real()), to_fp16(a_s[m].imag())};

    // ── 4. Sync Buffers to Device ────────────────────────────
    bo_X.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    bo_as.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    // ── 5. Execute Kernel ────────────────────────────────────
    printf("[HOST] Launching kernel...\n");
    auto t0 = std::chrono::high_resolution_clock::now();

    auto run = kernel(bo_X, bo_as, bo_w);
    run.wait();

    auto t1 = std::chrono::high_resolution_clock::now();
    double elapsed_us = std::chrono::duration<double, std::micro>(t1-t0).count();
    printf("[HOST] Kernel done in %.1f µs\n", elapsed_us);

    // ── 6. Sync Output ───────────────────────────────────────
    bo_w.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    cfp16* w_ptr = bo_w.map<cfp16*>();

    // ── 7. Print Weights ─────────────────────────────────────
    printf("\n=== Beamforming Weights ===\n");
    std::complex<float> dot(0,0);
    for (int m = 0; m < M_; m++) {
        float wr = from_fp16(w_ptr[m].re);
        float wi = from_fp16(w_ptr[m].im);
        printf("  w[%d] = %+.6f %+.6fi\n", m, wr, wi);
        // accumulate w^H * a_s
        dot += std::conj(std::complex<float>(wr,wi)) * a_s[m];
    }

    printf("\nDistortionless: w^H * a_s = (%.4f, %.4f)\n",
           dot.real(), dot.imag());
    float err = std::abs(dot - std::complex<float>(1.f,0.f));
    printf("Error: %.4f %s\n", err, err < 0.1f ? "[PASS]" : "[FAIL]");

    return (err < 0.1f) ? 0 : 1;
}
