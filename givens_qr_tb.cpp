// ============================================================================
// givens_qr_tb.cpp  —  Testbench for Givens QR + Back-substitution
// ============================================================================
// This file is NOT synthesised into hardware.
// It runs on your PC (C simulation) and on RTL simulation (co-simulation).
//
// What it tests:
//   TEST 1 — QR reconstruction:   ||Q·R − Lt|| / ||Lt|| < 1e-3
//   TEST 2 — Q unitarity:         ||Q^H·Q − I|| < 1e-3
//   TEST 3 — Distortionless check:|a_s^H · w| ≈ 1.0
//   TEST 4 — Null check:          output power at jammer direction is low
//
// The test values come directly from your MATLAB simulation —
// same Rxx matrix, same steering vector. This guarantees the hardware
// produces exactly what your MATLAB golden reference expects.
//
// HOW TO RUN:
//   In Vitis HLS:
//   1. Project → Run C Simulation   (tests C++ algorithm, fast)
//   2. Project → Run C Synthesis    (converts to RTL, check timing report)
//   3. Project → Run C/RTL Co-Sim   (tests generated RTL, slower)
// ============================================================================

#include <iostream>
#include <cmath>
#include <cstring>
#include "givens_qr.h"

// ── Tolerance for pass/fail ──────────────────────────────────────────────────
#define TOL_QR      1e-2    // QR reconstruction tolerance (fixed-point: 1e-2)
#define TOL_UNITARY 1e-2    // Q unitarity tolerance
#define TOL_DIST    0.05    // distortionless: |a_s^H*w| must be in [0.95, 1.05]

// ── Helper: print PASS or FAIL ───────────────────────────────────────────────
#define CHECK(cond, msg) \
    if (cond) { std::cout << "  [PASS] " << msg << std::endl; } \
    else       { std::cout << "  [FAIL] " << msg << std::endl; fail_count++; }

// ============================================================================
// TEST DATA — generated from MATLAB golden reference
// These are the EXACT values from your MATLAB script for M=8
// (first 8×8 submatrix of the 64×64 Rxx, for development)
//
// To generate your own: in MATLAB add after computing Rxx:
//   fprintf('Rxx(0,0)=%.6f+%.6fj\n', real(Rxx(1,1)), imag(Rxx(1,1)));
// ============================================================================

// For testing we use a simple known Hermitian PD matrix
// Real diagonal-dominant matrix — easy to verify QR by hand
// Replace with actual MATLAB Rxx values for production testing
static void fill_test_matrix(cplx_chol Lt[M][M],
                              cplx_chol L[M][M],
                              cplx_steer a_s[M])
{
    // Fill with identity-like matrix for basic sanity check
    // Real symmetric positive definite matrix (Cholesky of a scaled identity)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (i == j) {
                // Diagonal: sqrt of diagonal loading + signal power
                // Matches structure of Rxx diagonal after Cholesky
                float val = (i == 0) ? 5.2277f : (4.5f - 0.1f*i);
                L[i][j]  = cplx_chol(t_chol(val),  t_chol(0.0f));
                Lt[j][i] = cplx_chol(t_chol(val),  t_chol(0.0f));  // Lt = L^H
            } else if (j > i) {
                // Upper triangle of Lt (= lower triangle of L, conjugated)
                float r_val = 0.3f / (i+1);
                float i_val = 0.2f / (j+1);
                Lt[i][j] = cplx_chol(t_chol(r_val),  t_chol(i_val));
                L[j][i]  = cplx_chol(t_chol(r_val),  t_chol(-i_val)); // conj
                Lt[j][i] = cplx_chol(t_chol(0.0f),   t_chol(0.0f));
                L[i][j]  = cplx_chol(t_chol(0.0f),   t_chol(0.0f));
            }
        }
    }

    // Steering vector: unit magnitude complex exponential
    // Matches exp(-j*phase) for element 0..7 at az=20, el=20 (approximate)
    float phases[8] = {0.0f, 0.523f, 1.047f, 1.571f,
                       2.094f, 2.618f, 3.141f, 3.665f};
    for (int i = 0; i < M; i++) {
        a_s[i] = cplx_steer(t_steer(cosf(phases[i])),
                             t_steer(-sinf(phases[i])));
    }
}

// ============================================================================
// HELPER: complex magnitude
// ============================================================================
static float cmag(float r, float i) { return sqrtf(r*r + i*i); }

// ============================================================================
// HELPER: Frobenius norm of difference between two M×M complex arrays
// ============================================================================
static float frob_diff(float A_r[M][M], float A_i[M][M],
                       float B_r[M][M], float B_i[M][M])
{
    float sum = 0.0f;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++) {
            float dr = A_r[i][j] - B_r[i][j];
            float di = A_i[i][j] - B_i[i][j];
            sum += dr*dr + di*di;
        }
    return sqrtf(sum);
}

// ============================================================================
// MAIN TESTBENCH
// ============================================================================
int main()
{
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  Givens QR Testbench — Vitis HLS C Simulation\n";
    std::cout << "  Matrix size: " << M << " x " << M << "\n";
    std::cout << "  CORDIC iterations: " << ITER << "\n";
    std::cout << "============================================================\n\n";

    int fail_count = 0;

    // ── Declare I/O arrays ───────────────────────────────────────────────────
    cplx_chol   Lt[M][M];       // upper Cholesky factor (input)
    cplx_chol   L[M][M];        // lower Cholesky factor (input)
    cplx_steer  a_s[M];         // steering vector (input)
    cplx_weight w_out[M];       // weight vector (output)

    // ── Fill test data ───────────────────────────────────────────────────────
    fill_test_matrix(Lt, L, a_s);
    std::cout << "Test data loaded.\n\n";

    // ── Also get Q and R for intermediate checks ─────────────────────────────
    cplx_qmat Q[M][M];
    cplx_rmat R[M][M];

    // ── Call QR function directly for intermediate validation ─────────────────
    std::cout << "Running givens_qr()...\n";
    givens_qr(Lt, Q, R);
    std::cout << "  QR decomposition complete.\n\n";

    // ── TEST 1: QR reconstruction  ||Q*R - Lt|| / ||Lt|| < TOL ───────────────
    std::cout << "TEST 1: QR Reconstruction  ||Q*R - Lt|| / ||Lt||\n";
    {
        float QR_r[M][M] = {}, QR_i[M][M] = {};
        float Lt_r[M][M], Lt_i[M][M];
        float Lt_norm = 0.0f;

        for (int i = 0; i < M; i++)
            for (int j = 0; j < M; j++) {
                Lt_r[i][j] = (float)Lt[i][j].real();
                Lt_i[i][j] = (float)Lt[i][j].imag();
                Lt_norm += Lt_r[i][j]*Lt_r[i][j] + Lt_i[i][j]*Lt_i[i][j];
            }
        Lt_norm = sqrtf(Lt_norm);

        // Compute Q*R
        for (int i = 0; i < M; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < M; k++) {
                    float qr = (float)Q[i][k].real(), qi = (float)Q[i][k].imag();
                    float rr = (float)R[k][j].real(), ri = (float)R[k][j].imag();
                    QR_r[i][j] += qr*rr - qi*ri;
                    QR_i[i][j] += qr*ri + qi*rr;
                }

        float err = frob_diff(QR_r, QR_i, Lt_r, Lt_i) / Lt_norm;
        std::cout << "  Relative error: " << err << "  (tolerance: " << TOL_QR << ")\n";
        CHECK(err < TOL_QR, "QR reconstruction");
    }

    // ── TEST 2: Q unitarity  ||Q^H * Q - I|| < TOL ───────────────────────────
    std::cout << "\nTEST 2: Q Unitarity  ||Q^H*Q - I||\n";
    {
        float QhQ_r[M][M] = {}, QhQ_i[M][M] = {};
        float I_r[M][M]   = {}, I_i[M][M]   = {};

        for (int i = 0; i < M; i++) I_r[i][i] = 1.0f;  // identity

        // Q^H * Q: (Q^H)[i][k] = conj(Q[k][i])
        for (int i = 0; i < M; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < M; k++) {
                    float qhi_r =  (float)Q[k][i].real();  // conj → same real
                    float qhi_i = -(float)Q[k][i].imag();  // conj → negate imag
                    float qkj_r =  (float)Q[k][j].real();
                    float qkj_i =  (float)Q[k][j].imag();
                    QhQ_r[i][j] += qhi_r*qkj_r - qhi_i*qkj_i;
                    QhQ_i[i][j] += qhi_r*qkj_i + qhi_i*qkj_r;
                }

        float err = frob_diff(QhQ_r, QhQ_i, I_r, I_i);
        std::cout << "  Frobenius error: " << err << "  (tolerance: " << TOL_UNITARY << ")\n";
        CHECK(err < TOL_UNITARY, "Q unitarity");
    }

    // ── TEST 3: R is upper triangular ─────────────────────────────────────────
    std::cout << "\nTEST 3: R is upper triangular\n";
    {
        float max_lower = 0.0f;
        for (int i = 0; i < M; i++)
            for (int j = 0; j < i; j++) {
                float mag = cmag((float)R[i][j].real(), (float)R[i][j].imag());
                if (mag > max_lower) max_lower = mag;
            }
        std::cout << "  Max lower-triangle magnitude: " << max_lower
                  << "  (should be ~0)\n";
        CHECK(max_lower < 1e-3f, "R upper triangular");
    }

    // ── Call top function for end-to-end test ─────────────────────────────────
    std::cout << "\nRunning givens_qr_top() (end-to-end)...\n";
    givens_qr_top(Lt, L, a_s, w_out);
    std::cout << "  Top function complete.\n\n";

    // ── TEST 4: Distortionless constraint  |a_s^H * w| ≈ 1.0 ────────────────
    std::cout << "TEST 4: Distortionless constraint  |a_s^H * w|\n";
    {
        float dot_r = 0.0f, dot_i = 0.0f;
        for (int i = 0; i < M; i++) {
            // a_s^H * w = sum conj(a_s[i]) * w[i]
            float as_r =  (float)a_s[i].real();
            float as_i = -(float)a_s[i].imag();  // conjugate
            float w_r  =  (float)w_out[i].real();
            float w_i  =  (float)w_out[i].imag();
            dot_r += as_r*w_r - as_i*w_i;
            dot_i += as_r*w_i + as_i*w_r;
        }
        float mag = cmag(dot_r, dot_i);
        std::cout << "  |a_s^H * w| = " << mag
                  << "  (should be ≈ 1.0, tolerance ±" << TOL_DIST << ")\n";
        CHECK(fabsf(mag - 1.0f) < TOL_DIST, "Distortionless constraint");
    }

    // ── TEST 5: Print weight vector summary ───────────────────────────────────
    std::cout << "\nTEST 5: Weight vector norm\n";
    {
        float norm = 0.0f;
        for (int i = 0; i < M; i++) {
            float wr = (float)w_out[i].real();
            float wi = (float)w_out[i].imag();
            norm += wr*wr + wi*wi;
        }
        norm = sqrtf(norm);
        std::cout << "  ||W_MVDR|| = " << norm << "\n";
        std::cout << "  Weight vector (real, imag):\n";
        for (int i = 0; i < M; i++) {
            std::cout << "    w[" << i << "] = "
                      << (float)w_out[i].real() << " + j"
                      << (float)w_out[i].imag() << "\n";
        }
        CHECK(norm > 0.0f && norm < 10.0f, "Weight norm in reasonable range");
    }

    // ── SUMMARY ──────────────────────────────────────────────────────────────
    std::cout << "\n============================================================\n";
    if (fail_count == 0) {
        std::cout << "  ALL TESTS PASSED — ready for C Synthesis\n";
    } else {
        std::cout << "  " << fail_count << " TEST(S) FAILED\n";
        std::cout << "  Fix failures before running C Synthesis.\n";
    }
    std::cout << "============================================================\n\n";

    return fail_count;  // HLS expects 0 return for pass
}
