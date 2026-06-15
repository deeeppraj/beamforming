// =============================================================================
//  tb_covariance_matrix.cpp
//  C-simulation testbench for covariance_estimation kernel
//  Tool : Vitis HLS 2024.1  (run via C Simulation, not standalone g++)
//
//  Test strategy
//    Test 1 – Identity check
//      X = I_M (first M columns of identity, rest zero snapshots).
//      Expected R = (1/K) * I_M.
//      Checks diagonal real = 1/K, off-diagonal = 0, all imag = 0.
//
//    Test 2 – Rank-1 coherent signal
//      All K snapshots are the same complex vector v (all antennas, same value).
//      X[:,t] = v  for t = 0..K-1.
//      R_hat = v * v^H  (rank-1, all entries equal v_i * conj(v_j)).
//      Checks Hermitian symmetry, diagonal real, off-diagonal cross terms.
//
//    Test 3 – Hermitian symmetry sweep
//      Random (deterministic seed) complex X.  After calling the kernel,
//      verifies R[i][j] == conj(R[j][i]) for all i,j and R[i][i].imag == 0.
//
//  Pass/fail: each test prints PASS or FAIL with the violating entry.
//  Return value: 0 = all pass, non-zero = failure count.
// =============================================================================

#include "covariance_matrix.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

// ─── Tolerance for fixed-point comparison ────────────────────────────────────
// Fixed-point Q8.24 resolution ≈ 2^-24 ≈ 6e-8; allow 10× for rounding budget.
static constexpr double TOL = 1e-3;   // generous: accounts for Q4.12 input quantisation

// ─── Helper: absolute value of a double ──────────────────────────────────────
static inline double dabs(double x) { return x < 0.0 ? -x : x; }

// ─── Forward declaration of DUT ──────────────────────────────────────────────
// (already in covariance_matrix.h but repeated here for clarity)
// void covariance_estimation(csamp_t X[M_ANT][K_SAMP], ccov_result_t R[M_ANT][M_ANT]);

// ─────────────────────────────────────────────────────────────────────────────
//  Utility: zero-fill input and output arrays
// ─────────────────────────────────────────────────────────────────────────────
static void zero_X(csamp_t X[M_ANT][K_SAMP])
{
    for (int i = 0; i < M_ANT; i++)
        for (int t = 0; t < K_SAMP; t++) {
            X[i][t].re = samp_t(0);
            X[i][t].im = samp_t(0);
        }
}

static void zero_R(ccov_result_t R[M_ANT][M_ANT])
{
    for (int i = 0; i < M_ANT; i++)
        for (int j = 0; j < M_ANT; j++) {
            R[i][j].re = cov_result_t(0);
            R[i][j].im = cov_result_t(0);
        }
}

// ─────────────────────────────────────────────────────────────────────────────
//  TEST 1 – Identity check
//
//  X[i][0] = delta_{i,0} … X[i][M_ANT-1] = delta_{i, M_ANT-1}
//  remaining snapshots are zero.
//  R_hat = (1/K) * I_M  →  R[i][i].re = 1/K,  R[i][j]=0 for i≠j
// ─────────────────────────────────────────────────────────────────────────────
static int test_identity()
{
    printf("\n=== TEST 1: Identity input ===\n");

    static csamp_t      X[M_ANT][K_SAMP];
    static ccov_result_t R[M_ANT][M_ANT];

    zero_X(X);
    zero_R(R);

    // X[:,t] = e_t  for t = 0..M_ANT-1  (unit impulse on antenna t, snap t)
    for (int t = 0; t < M_ANT; t++) {
        X[t][t].re = samp_t(1);
        X[t][t].im = samp_t(0);
    }

    covariance_estimation(X, R);

    const double expected_diag = 1.0 / K_SAMP;   // 1/129 ≈ 0.007752
    int fails = 0;

    for (int i = 0; i < M_ANT; i++) {
        for (int j = 0; j < M_ANT; j++) {
            double got_re = (double)R[i][j].re;
            double got_im = (double)R[i][j].im;

            if (i == j) {
                // Diagonal real should equal 1/K
                if (dabs(got_re - expected_diag) > TOL) {
                    printf("  FAIL R[%d][%d].re = %.6f, expected %.6f\n",
                           i, j, got_re, expected_diag);
                    fails++;
                }
                // Diagonal imag should be exactly zero
                if (dabs(got_im) > TOL) {
                    printf("  FAIL R[%d][%d].im = %.6f, expected 0\n", i, j, got_im);
                    fails++;
                }
            } else {
                // Off-diagonal should be zero
                if (dabs(got_re) > TOL || dabs(got_im) > TOL) {
                    printf("  FAIL R[%d][%d] = (%.6f, %.6f), expected (0, 0)\n",
                           i, j, got_re, got_im);
                    fails++;
                    if (fails > 5) { printf("  ... (further failures suppressed)\n"); goto done1; }
                }
            }
        }
    }
    done1:
    if (fails == 0) printf("  PASS (diagonal = %.6f, off-diagonal = 0)\n", expected_diag);
    return fails;
}

// ─────────────────────────────────────────────────────────────────────────────
//  TEST 2 – Rank-1 coherent signal
//
//  All K snapshots identical: X[i][t] = v[i] for all t.
//  R_hat = (1/K) * K * v * v^H = v * v^H  (unit amplitude → diag = |v_i|^2)
//  Here v[i] = (cos(i*0.1), sin(i*0.1)) — unit-magnitude complex vector.
// ─────────────────────────────────────────────────────────────────────────────
static int test_rank1()
{
    printf("\n=== TEST 2: Rank-1 coherent signal ===\n");

    static csamp_t       X[M_ANT][K_SAMP];
    static ccov_result_t R[M_ANT][M_ANT];

    zero_X(X);
    zero_R(R);

    // Build unit-magnitude steering-like vector
    double v_re[M_ANT], v_im[M_ANT];
    for (int i = 0; i < M_ANT; i++) {
        double phase = i * 0.1;                   // phase ramp
        v_re[i] = cos(phase);
        v_im[i] = sin(phase);
    }

    // Fill all K snapshots with the same vector
    for (int i = 0; i < M_ANT; i++) {
        samp_t sr = samp_t(v_re[i]);
        samp_t si = samp_t(v_im[i]);
        for (int t = 0; t < K_SAMP; t++) {
            X[i][t].re = sr;
            X[i][t].im = si;
        }
    }

    covariance_estimation(X, R);

    // Expected: R[i][j] = v[i] * conj(v[j])  (since K samples × (1/K) = 1)
    int fails = 0;
    for (int i = 0; i < M_ANT && fails <= 5; i++) {
        for (int j = 0; j < M_ANT && fails <= 5; j++) {
            // v[i] * conj(v[j]) = (vr_i + j*vi_i)(vr_j - j*vi_j)
            double exp_re =  v_re[i]*v_re[j] + v_im[i]*v_im[j];
            double exp_im =  v_im[i]*v_re[j] - v_re[i]*v_im[j];
            double got_re = (double)R[i][j].re;
            double got_im = (double)R[i][j].im;

            if (dabs(got_re - exp_re) > TOL || dabs(got_im - exp_im) > TOL) {
                printf("  FAIL R[%d][%d] = (%.5f, %.5f), expected (%.5f, %.5f)\n",
                       i, j, got_re, got_im, exp_re, exp_im);
                fails++;
            }
        }
    }
    if (fails == 0) printf("  PASS (rank-1 outer product verified for all %d×%d entries)\n",
                           M_ANT, M_ANT);
    return fails;
}

// ─────────────────────────────────────────────────────────────────────────────
//  TEST 3 – Hermitian symmetry check on pseudo-random input
//
//  Uses a simple deterministic LCG to generate X entries.
//  Verifies: R[i][j].re == R[j][i].re  and  R[i][j].im == -R[j][i].im
//            R[i][i].im == 0  for all i
// ─────────────────────────────────────────────────────────────────────────────
static int test_hermitian()
{
    printf("\n=== TEST 3: Hermitian symmetry on random input ===\n");

    static csamp_t       X[M_ANT][K_SAMP];
    static ccov_result_t R[M_ANT][M_ANT];

    zero_X(X);
    zero_R(R);

    // Deterministic LCG (seed=42)
    unsigned int rng = 42;
    for (int i = 0; i < M_ANT; i++) {
        for (int t = 0; t < K_SAMP; t++) {
            rng = rng * 1664525u + 1013904223u;
            double re = ((int)(rng & 0xFFFF) - 32768) / 32768.0 * 4.0;  // ±4
            rng = rng * 1664525u + 1013904223u;
            double im = ((int)(rng & 0xFFFF) - 32768) / 32768.0 * 4.0;
            X[i][t].re = samp_t(re);
            X[i][t].im = samp_t(im);
        }
    }

    covariance_estimation(X, R);

    int fails = 0;

    // Check R[i][j] == conj(R[j][i])
    for (int i = 0; i < M_ANT && fails <= 10; i++) {
        for (int j = i+1; j < M_ANT && fails <= 10; j++) {
            double re_ij  = (double)R[i][j].re;
            double im_ij  = (double)R[i][j].im;
            double re_ji  = (double)R[j][i].re;
            double im_ji  = (double)R[j][i].im;

            if (dabs(re_ij - re_ji) > TOL || dabs(im_ij + im_ji) > TOL) {
                printf("  FAIL Hermitian: R[%d][%d]=(%.5f,%.5f) vs R[%d][%d]=(%.5f,%.5f)\n",
                       i, j, re_ij, im_ij, j, i, re_ji, im_ji);
                fails++;
            }
        }
    }

    // Check diagonal imaginary = 0
    for (int i = 0; i < M_ANT && fails <= 10; i++) {
        double d_im = (double)R[i][i].im;
        if (dabs(d_im) > TOL) {
            printf("  FAIL R[%d][%d].im = %.8f (expected 0)\n", i, i, d_im);
            fails++;
        }
    }

    if (fails == 0) printf("  PASS (Hermitian symmetry verified, all diagonal imag = 0)\n");
    return fails;
}

// ─────────────────────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────────────────────
int main()
{
    printf("============================================================\n");
    printf(" Covariance Matrix Kernel – C-Simulation Testbench\n");
    printf(" M_ANT=%d  K_SAMP=%d  Tolerance=%.1e\n", M_ANT, K_SAMP, TOL);
    printf("============================================================\n");

    int total_fails = 0;

    total_fails += test_identity();
    total_fails += test_rank1();
    total_fails += test_hermitian();

    printf("\n============================================================\n");
    if (total_fails == 0)
        printf(" ALL TESTS PASSED\n");
    else
        printf(" FAILED: %d violation(s) detected\n", total_fails);
    printf("============================================================\n");

    return total_fails;
}
