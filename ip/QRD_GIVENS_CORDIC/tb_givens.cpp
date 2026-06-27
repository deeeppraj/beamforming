//============================================================================
// tb_givens.cpp — Testbench for MVDR Givens QR + Cholesky + Back-Sub IP
// ============================================================================
// Test coverage:
//   Test 1: QR decomposition properties (R upper-triangular, Q*Q^H = I)
//   Test 2: Back-substitution with diagonal L — known exact solution
//   Test 3: Full top function with DIAGONAL Rxx (original test, fast check)
//   Test 4: Full top function with DENSE Rxx — exercises the off-diagonal
//           path in cholesky_core (sum_off / cplx_accum), which Test 3 skips.
//           Prints full input Rxx, steering vector, and output weights.
// ============================================================================

#include "givens.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

static constexpr double TOL_QR = 5e-2;
static constexpr double TOL_W  = 5e-2;

static inline double dabs(double x) { return x < 0.0 ? -x : x; }

// ---------------------------------------------------------------------------
// Helper: print a complex number cleanly
// ---------------------------------------------------------------------------
static void print_cplx(double re, double im) {
    if (im >= 0.0) printf("%+.6f + %.6fj", re, im);
    else           printf("%+.6f - %.6fj", re, dabs(im));
}

// ---------------------------------------------------------------------------
// Build an M×M upper-triangular test matrix (values safe for t_chol range)
// Used by Test 1 (QR decomposition property check)
// ---------------------------------------------------------------------------
static void build_test_matrix(cplx_chol A[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (j < i) {
                A[i][j] = cplx_chol(t_chol(0), t_chol(0));
            } else if (i == j) {
                // Diagonal in [1, 3] — safe for t_chol<16,5> and t_cordic<32,6>
                double diag = 1.0 + (double)i * (2.0 / M);
                A[i][j] = cplx_chol(t_chol(diag), t_chol(0));
            } else {
                // Off-diagonal small, decaying with distance
                double scale = 0.1 / (j - i + 1);
                A[i][j] = cplx_chol(t_chol(scale), t_chol(scale * 0.5));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Build a DENSE positive-definite Rxx = A*A^H where A is a scaled random-like
// matrix.  Deterministic via a simple LCG so results are reproducible.
// The construction guarantees Rxx is Hermitian PD, giving cholesky_core
// a valid non-diagonal input that fully exercises the off-diagonal code path.
// ---------------------------------------------------------------------------
static void build_dense_rxx(cplx_rxx Rxx[M][M]) {
    // LCG parameters (Knuth)
    unsigned long long seed = 123456789ULL;
    auto lcg_next = [&]() -> double {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        return (double)(int)(seed >> 33) / (double)(1 << 30);  // in (-2, 2)
    };

    // Factor matrix A: diagonal ~2, off-diagonal ~0.05 (ensures dominance)
    double Ar[M][M], Ai[M][M];
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (i == j) {
                Ar[i][j] = 2.0;
                Ai[i][j] = 0.0;
            } else {
                Ar[i][j] = lcg_next() * 0.05;
                Ai[i][j] = lcg_next() * 0.05;
            }
        }
    }

    // Rxx = A * A^H  (Hermitian PD by construction)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            double re = 0.0, im = 0.0;
            for (int k = 0; k < M; k++) {
                // (A[i][k]) * conj(A[j][k])
                re += Ar[i][k]*Ar[j][k] + Ai[i][k]*Ai[j][k];
                im += Ai[i][k]*Ar[j][k] - Ar[i][k]*Ai[j][k];
            }
            Rxx[i][j] = cplx_rxx(t_rxx(re), t_rxx(im));
        }
    }
}

// ---------------------------------------------------------------------------
// TEST 1: QR decomposition properties
//   Pass criteria:
//   (a) R[i][j] ≈ 0 for all i > j  (upper triangular)
//   (b) (Q * Q^H)[i][j] ≈ I[i][j]  (unitary Q)
// ---------------------------------------------------------------------------
static int test_qr_properties() {
    printf("\n=== TEST 1: QR decomposition properties (M=%d) ===\n", M);
    static cplx_chol A[M][M];
    static cplx_qmat Q[M][M];
    static cplx_rmat R[M][M];
    build_test_matrix(A);

    givens_qr(A, Q, R);

    int fails = 0;

    // (a) R upper-triangular
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < i; j++) {
            double r_re = (double)R[i][j].real();
            double r_im = (double)R[i][j].imag();
            if (dabs(r_re) > TOL_QR || dabs(r_im) > TOL_QR) {
                if (fails < 3)
                    printf("  [R upper-tri] R[%d][%d] = (%.4f, %.4f) — expected ≈0\n",
                           i, j, r_re, r_im);
                fails++;
            }
        }
    }

    // (b) Q * Q^H = I
    for (int i = 0; i < M && fails <= 10; i++) {
        for (int j = 0; j < M && fails <= 10; j++) {
            double acc_re = 0.0, acc_im = 0.0;
            for (int k = 0; k < M; k++) {
                double qik_r = (double)Q[i][k].real(), qik_i = (double)Q[i][k].imag();
                double qjk_r = (double)Q[j][k].real(), qjk_i = (double)Q[j][k].imag();
                // Q[i][k] * conj(Q[j][k])
                acc_re += qik_r*qjk_r + qik_i*qjk_i;
                acc_im += qik_i*qjk_r - qik_r*qjk_i;
            }
            double exp_re = (i == j) ? 1.0 : 0.0;
            if (dabs(acc_re - exp_re) > TOL_QR || dabs(acc_im) > TOL_QR) {
                if (fails < 3)
                    printf("  [Q*Q^H] [%d][%d] = (%.4f, %.4f) — expected (%.1f, 0)\n",
                           i, j, acc_re, acc_im, exp_re);
                fails++;
            }
        }
    }

    printf("  Result: %s (%d violation(s))\n", (fails==0) ? "PASS" : "FAIL", fails);
    return fails;
}

// ---------------------------------------------------------------------------
// TEST 2: Back-substitution with diagonal L, known exact solution
//   Rxx diagonal: Rxx[i][i] = l_diag^2, so L[i][i] = l_diag
//   Steering: a_s = [1, 0, 0, ..., 0]
//   Expected: w[0] = 1/l_diag^2 = 1.0, w[i>0] = 0
// ---------------------------------------------------------------------------
static int test_back_substitution() {
    printf("\n=== TEST 2: Back-substitution with diagonal L ===\n");
    static cplx_chol  L[M][M], Lt[M][M];
    static cplx_steer a_s[M];
    static cplx_qmat  Q[M][M];
    static cplx_rmat  R[M][M];
    static cplx_weight w[M];

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            L[i][j] = cplx_chol(0,0); Lt[i][j] = cplx_chol(0,0);
        }
        double dv = 1.0 + i * (2.0 / M);
        L[i][i]  = cplx_chol(t_chol(dv), t_chol(0));
        Lt[i][i] = cplx_chol(t_chol(dv), t_chol(0));
        a_s[i]   = cplx_steer(t_steer(i==0 ? 1.0 : 0.0), t_steer(0));
    }
    givens_qr(Lt, Q, R);
    back_substitution(Q, R, L, a_s, w);

    // w[0] should equal 1/L[0][0]^2 = 1/(1.0)^2 = 1.0
    double w0_ref = 1.0 / (1.0 * 1.0);
    int fails = 0;

    printf("  w[0] = (%.6f, %.6f) — ref %.6f\n",
           (double)w[0].real(), (double)w[0].imag(), w0_ref);
    if (dabs((double)w[0].real() - w0_ref) > TOL_W) fails++;

    for (int i = 1; i < M && fails <= 5; i++) {
        double wr = (double)w[i].real(), wi = (double)w[i].imag();
        if (dabs(wr) > TOL_W || dabs(wi) > TOL_W) {
            if (fails < 3)
                printf("  w[%d] = (%.6f, %.6f) — expected ≈0\n", i, wr, wi);
            fails++;
        }
    }

    printf("  Result: %s (%d violation(s))\n", (fails==0) ? "PASS" : "FAIL", fails);
    return fails;
}

// ---------------------------------------------------------------------------
// TEST 3: givens_qr_top with DIAGONAL Rxx (fast sanity check)
// ---------------------------------------------------------------------------
static int test_top_diagonal() {
    printf("\n=== TEST 3: givens_qr_top — diagonal Rxx ===\n");
    static cplx_rxx    Rxx[M][M];
    static cplx_steer  a_s[M];
    static cplx_weight w_out[M];

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) Rxx[i][j] = cplx_rxx(0, 0);
        double l_d = 1.0 + i * (2.0 / M);
        Rxx[i][i] = cplx_rxx(t_rxx(l_d * l_d), t_rxx(0));
        a_s[i]    = cplx_steer(t_steer(i==0 ? 1.0 : 0.0), t_steer(0));
    }

    printf("  Steering vector a_s (first 4 of %d):\n", M);
    for (int i = 0; i < 4; i++) {
        printf("    a_s[%d] = (%.4f, %.4f)\n",
               i, (double)a_s[i].real(), (double)a_s[i].imag());
    }
    printf("  Rxx diagonal (first 4):\n");
    for (int i = 0; i < 4; i++) {
        printf("    Rxx[%d][%d] = %.6f\n", i, i, (double)Rxx[i][i].real());
    }

    givens_qr_top(Rxx, a_s, w_out);

    printf("  Output weights (first 8 of %d):\n", M);
    for (int i = 0; i < 8; i++) {
        printf("    w[%d] = ", i);
        print_cplx((double)w_out[i].real(), (double)w_out[i].imag());
        printf("\n");
    }

    double w0_ref = 1.0;
    int fails = 0;
    if (dabs((double)w_out[0].real() - w0_ref) > TOL_W) fails++;
    for (int i = 1; i < M && fails <= 5; i++) {
        if (dabs((double)w_out[i].real()) > TOL_W) fails++;
    }
    printf("  Result: %s (%d violation(s))\n", (fails==0) ? "PASS" : "FAIL", fails);
    return fails;
}

// ---------------------------------------------------------------------------
// TEST 4: givens_qr_top with DENSE Rxx — full off-diagonal code path
//
// This is the critical new test. The dense Rxx = A*A^H exercises:
//   • cholesky_core's OFF_DIAG_DOT loop (sum_off, cplx_accum)
//   • The [FIX-1] t_cordic saturation fix (larger L diagonal values)
//   • The [FIX-2] t_accum integer width fix
//   • The [FIX-3] direct ap_fixed sqrt path
//
// Manual verification approach:
//   1. From the printed Rxx, compute L = chol(Rxx) in MATLAB/Python.
//   2. Solve w = Rxx^{-1} * a_s / (a_s^H * Rxx^{-1} * a_s) (unnormalized: w = Rxx^{-1}*a_s).
//   3. Compare with printed w_out. Tolerance is TOL_W = 5e-2 in fixed-point.
// ---------------------------------------------------------------------------
static int test_top_dense() {
    printf("\n=== TEST 4: givens_qr_top — DENSE Rxx (off-diagonal path) ===\n");
    static cplx_rxx    Rxx[M][M];
    static cplx_steer  a_s[M];
    static cplx_weight w_out[M];

    build_dense_rxx(Rxx);

    // After build_dense_rxx(Rxx), print full matrix for MATLAB import
    printf("\n  === FULL Rxx (MATLAB paste) ===\n");
    printf("Rxx = [\n");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            printf("  %.8f%+.8fj", (double)Rxx[i][j].real(),
                                    (double)Rxx[i][j].imag());
            if (j < M-1) printf(",");
        }
        printf(";\n");
    }
    printf("];\n");

    // Steering vector: uniform phase progression (simulates boresight beam)
    // a_s[i] = exp(j * 0.1 * i) — magnitude 1, bounded in t_steer<16,2>
    for (int i = 0; i < M; i++) {
        double ph = 0.1 * i;
        a_s[i] = cplx_steer(t_steer(cos(ph)), t_steer(sin(ph)));
    }

    // ---- Print inputs ----
    printf("\n  === INPUT: Rxx (first 4x4 block) ===\n");
    for (int i = 0; i < 4; i++) {
        printf("  Row %d: ", i);
        for (int j = 0; j < 4; j++) {
            printf("[");
            print_cplx((double)Rxx[i][j].real(), (double)Rxx[i][j].imag());
            printf("] ");
        }
        printf("\n");
    }

    printf("\n  === INPUT: Rxx diagonal (all %d elements) ===\n", M);
    for (int i = 0; i < M; i++) {
        printf("  Rxx[%2d][%2d] = %+.6f\n", i, i, (double)Rxx[i][i].real());
    }

    printf("\n  === INPUT: Steering vector a_s (all %d elements) ===\n", M);
    for (int i = 0; i < M; i++) {
        printf("  a_s[%2d] = ", i);
        print_cplx((double)a_s[i].real(), (double)a_s[i].imag());
        printf("\n");
    }

    // ---- Run DUT ----
    givens_qr_top(Rxx, a_s, w_out);

    // ---- Print outputs ----
    printf("\n  === OUTPUT: Weight vector w (all %d elements) ===\n", M);
    for (int i = 0; i < M; i++) {
        printf("  w[%2d] = ", i);
        print_cplx((double)w_out[i].real(), (double)w_out[i].imag());
        printf("\n");
    }

    // ---- Sanity check: |w[i]| should be small but not all zero ----
    // For this Rxx (diagonal-dominant ~4 on diagonal, off-diag ~0.1*M≈0.1*64=6.4
    // upper bounded), w should have reasonable magnitudes.
    // We check: at least one w[i] has |w[i]| > 1e-3 (not all-zero/saturated)
    // and all |w[i]| < 10 (not blown up).
    int fails = 0;
    double max_mag = 0.0;
    bool any_nonzero = false;

    for (int i = 0; i < M; i++) {
        double wr = (double)w_out[i].real();
        double wi = (double)w_out[i].imag();
        double mag = sqrt(wr*wr + wi*wi);
        if (mag > max_mag) max_mag = mag;
        if (mag > 1e-3) any_nonzero = true;
        if (mag > 10.0) {           // blown-up weight — saturation artifact
            if (fails < 3)
                printf("  [WARN] |w[%d]| = %.4f — suspiciously large\n", i, mag);
            fails++;
        }
    }
    if (!any_nonzero) {
        printf("  [FAIL] All weights are zero — likely saturation upstream\n");
        fails++;
    }
    printf("\n  Max |w[i]| = %.6f\n", max_mag);
    printf("  Result: %s (%d violation(s))\n", (fails==0) ? "PASS" : "FAIL", fails);
    return fails;
}

// ---------------------------------------------------------------------------
// MAIN
// ---------------------------------------------------------------------------
int main() {
    printf("============================================================\n");
    printf("  Givens QR + Cholesky + Back-Sub (Architecture B)\n");
    printf("  M=%d  ITER=%d  TOL_QR=%.1e  TOL_W=%.1e\n",
           M, ITER, TOL_QR, TOL_W);
    printf("============================================================\n");

    int total = 0;
    total += test_qr_properties();
    total += test_back_substitution();
    total += test_top_diagonal();
    total += test_top_dense();         // NEW: exercises off-diagonal Cholesky path

    printf("\n============================================================\n");
    if (total == 0) printf("  ALL TESTS PASSED\n");
    else            printf("  TOTAL FAILURES: %d violation(s)\n", total);
    printf("============================================================\n");

    return total;
}