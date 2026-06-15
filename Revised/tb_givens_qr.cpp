// ============================================================================
//  tb_givens_qr.cpp
//  C-simulation testbench for givens_qr_top (Givens QR + back-substitution)
//  Tool : Vitis HLS 2024.1
//
//  Tests
//    Test 1 – QR decomposition: A = Q*R
//      Applies givens_qr to a known 8×8 upper-triangular Cholesky-like matrix.
//      Verifies:
//        (a) Q*Q^H ≈ I   (Q is unitary)
//        (b) R is upper triangular (all subdiagonal entries ≈ 0)
//        (c) Q*R ≈ A_in  (reconstruction)
//
//    Test 2 – Back-substitution: solve Rxx·w = a_s
//      Uses a diagonal L (simplest non-trivial Cholesky).
//      Computes golden reference w = L^{-H} Q^H L^{-1} a_s in double precision.
//      Verifies |w_hw - w_ref| < TOL for all elements.
//
//    Test 3 – Top-level integration: givens_qr_top
//      Feeds the same inputs through the AXI-interfaced top function.
//      Verifies the weight vector matches Test 2 reference.
//
//  Return: 0 = all pass, else = failure count.
// ============================================================================

#include "givens_qr.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

// ─── Tolerance ───────────────────────────────────────────────────────────────
// t_cordic = ap_fixed<32,4> → resolution 2^-28 ≈ 3.7e-9.
// CORDIC introduces ≈ 2^-ITER = 2^-16 ≈ 1.5e-5 error per operation.
// Fixed-point types add ≈ 2^-12 at output cast.  Use 1e-2 for M=8.
static constexpr double TOL_QR  = 1e-2;    // for Q*R ≈ A, Q*Q^H ≈ I
static constexpr double TOL_W   = 1e-2;    // for weight vector

static inline double dabs(double x) { return x < 0.0 ? -x : x; }

// ─── Print a complex matrix (debug helper, only called on failure) ────────────
static void print_mat(const char *name, double re[M][M], double im[M][M])
{
    printf("  %s:\n", name);
    for (int i = 0; i < M; i++) {
        printf("    row %d: ", i);
        for (int j = 0; j < M; j++)
            printf("(%.4f,%.4f) ", re[i][j], im[i][j]);
        printf("\n");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Build a test input matrix A_in: upper triangular with known values.
//  Using A_in = diag(1,2,...,M) + 0.5*upper_shift so we have a non-trivial
//  upper triangular matrix that mimics a Cholesky factor Lt.
// ─────────────────────────────────────────────────────────────────────────────
static void build_test_matrix(cplx_chol A[M][M])
{
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (j < i) {
                A[i][j] = cplx_chol(t_chol(0), t_chol(0));
            } else if (i == j) {
                // Diagonal: real, positive (Cholesky property)
                A[i][j] = cplx_chol(t_chol(i+1), t_chol(0));
            } else {
                // Super-diagonal: small complex values
                double scale = 0.3 / (j - i + 1);
                A[i][j] = cplx_chol(t_chol(scale),
                                     t_chol(scale * 0.5));
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  TEST 1 – QR decomposition properties
// ─────────────────────────────────────────────────────────────────────────────
static int test_qr_properties()
{
    printf("\n=== TEST 1: QR decomposition properties ===\n");

    static cplx_chol A[M][M];
    static cplx_qmat Q[M][M];
    static cplx_rmat R[M][M];

    build_test_matrix(A);

    // Call givens_qr directly (not top function — no AXI overhead in C-sim)
    givens_qr(A, Q, R);

    int fails = 0;

    // ── (a) Check R is upper triangular ──────────────────────────────────────
    printf("  (a) Checking R is upper triangular...\n");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < i; j++) {   // sub-diagonal: should be ≈ 0
            double r_re = (double)R[i][j].real();
            double r_im = (double)R[i][j].imag();
            if (dabs(r_re) > TOL_QR || dabs(r_im) > TOL_QR) {
                printf("  FAIL R[%d][%d] = (%.5f, %.5f), expected 0\n",
                       i, j, r_re, r_im);
                fails++;
                if (fails > 5) goto done_tri;
            }
        }
    }
    done_tri:
    if (fails == 0) printf("    PASS\n");

    // ── (b) Check Q * Q^H = I ────────────────────────────────────────────────
    printf("  (b) Checking Q * Q^H = I ...\n");
    {
        int f_before = fails;
        for (int i = 0; i < M && fails - f_before <= 5; i++) {
            for (int j = 0; j < M && fails - f_before <= 5; j++) {
                // (Q * Q^H)[i][j] = Σ_k Q[i][k] * conj(Q[j][k])
                double acc_re = 0.0, acc_im = 0.0;
                for (int k = 0; k < M; k++) {
                    double qik_r = (double)Q[i][k].real();
                    double qik_i = (double)Q[i][k].imag();
                    double qjk_r = (double)Q[j][k].real();
                    double qjk_i = (double)Q[j][k].imag();
                    // Q[i][k] * conj(Q[j][k])
                    acc_re += qik_r*qjk_r + qik_i*qjk_i;
                    acc_im += qik_i*qjk_r - qik_r*qjk_i;
                }
                double exp_re = (i == j) ? 1.0 : 0.0;
                double exp_im = 0.0;
                if (dabs(acc_re - exp_re) > TOL_QR || dabs(acc_im - exp_im) > TOL_QR) {
                    printf("  FAIL (Q*Q^H)[%d][%d] = (%.5f,%.5f), expected (%.1f,0)\n",
                           i, j, acc_re, acc_im, exp_re);
                    fails++;
                }
            }
        }
        if (fails == f_before) printf("    PASS\n");
    }

    // ── (c) Check Q * R ≈ A_in ───────────────────────────────────────────────
    printf("  (c) Checking Q * R = A_in (reconstruction) ...\n");
    {
        int f_before = fails;
        for (int i = 0; i < M && fails - f_before <= 5; i++) {
            for (int j = 0; j < M && fails - f_before <= 5; j++) {
                // (Q * R)[i][j] = Σ_k Q[i][k] * R[k][j]
                double acc_re = 0.0, acc_im = 0.0;
                for (int k = 0; k < M; k++) {
                    double qik_r = (double)Q[i][k].real();
                    double qik_i = (double)Q[i][k].imag();
                    double rkj_r = (double)R[k][j].real();
                    double rkj_i = (double)R[k][j].imag();
                    acc_re += qik_r*rkj_r - qik_i*rkj_i;
                    acc_im += qik_r*rkj_i + qik_i*rkj_r;
                }
                double a_re = (double)A[i][j].real();
                double a_im = (double)A[i][j].imag();
                if (dabs(acc_re - a_re) > TOL_QR || dabs(acc_im - a_im) > TOL_QR) {
                    printf("  FAIL (Q*R)[%d][%d] = (%.5f,%.5f), A[%d][%d] = (%.5f,%.5f)\n",
                           i, j, acc_re, acc_im, i, j, a_re, a_im);
                    fails++;
                }
            }
        }
        if (fails == f_before) printf("    PASS\n");
    }

    return fails;
}

// ─────────────────────────────────────────────────────────────────────────────
//  TEST 2 – Back-substitution with diagonal L
//
//  L = diag(l_0, l_1, ..., l_{M-1})  all real positive
//  Lt = diag(l_0, l_1, ..., l_{M-1})  (same for diagonal case)
//  a_s = (1, 0, 0, ..., 0)^T  (unit vector along first antenna)
//
//  Golden reference (diagonal case is exact):
//    z = L^{-1} a_s    →  z[0] = 1/l_0, rest = 0
//    w = Lt^{-1} z     →  w[0] = 1/(l_0^2), rest = 0
//  (Since Lt = diag and Q for diagonal Lt is I after QR decomposition)
// ─────────────────────────────────────────────────────────────────────────────
static int test_back_substitution()
{
    printf("\n=== TEST 2: Back-substitution (diagonal L, unit steering) ===\n");

    static cplx_chol  L[M][M];
    static cplx_chol  Lt[M][M];
    static cplx_steer a_s[M];
    static cplx_qmat  Q[M][M];
    static cplx_rmat  R[M][M];
    static cplx_weight w[M];

    // Build diagonal L and Lt
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            L[i][j]  = cplx_chol(t_chol(0), t_chol(0));
            Lt[i][j] = cplx_chol(t_chol(0), t_chol(0));
        }
        double diag_val = 1.0 + i * 0.25;   // 1.0, 1.25, 1.5, ..., 2.75
        L[i][i]  = cplx_chol(t_chol(diag_val), t_chol(0));
        Lt[i][i] = cplx_chol(t_chol(diag_val), t_chol(0));
    }

    // Steering vector: e_0
    for (int i = 0; i < M; i++) {
        a_s[i] = cplx_steer(t_steer(i==0 ? 1.0 : 0.0), t_steer(0));
    }

    // Compute QR of Lt
    givens_qr(Lt, Q, R);

    // Call back_substitution
    back_substitution(Q, R, L, a_s, w);

    // Golden reference: w[0] = 1/(l_0^2), all others 0
    double l0 = 1.0;
    double w0_ref_re = 1.0 / (l0 * l0);
    double w0_ref_im = 0.0;

    int fails = 0;
    double w0_got_re = (double)w[0].real();
    double w0_got_im = (double)w[0].imag();

    printf("  w[0] expected (%.5f, %.5f), got (%.5f, %.5f)\n",
           w0_ref_re, w0_ref_im, w0_got_re, w0_got_im);

    if (dabs(w0_got_re - w0_ref_re) > TOL_W || dabs(w0_got_im - w0_ref_im) > TOL_W) {
        printf("  FAIL w[0]: error re=%.2e im=%.2e (TOL=%.1e)\n",
               dabs(w0_got_re - w0_ref_re), dabs(w0_got_im - w0_ref_im), TOL_W);
        fails++;
    }

    for (int i = 1; i < M && fails <= 5; i++) {
        double w_re = (double)w[i].real();
        double w_im = (double)w[i].imag();
        if (dabs(w_re) > TOL_W || dabs(w_im) > TOL_W) {
            printf("  FAIL w[%d] = (%.5f, %.5f), expected (0, 0)\n", i, w_re, w_im);
            fails++;
        }
    }

    if (fails == 0) printf("  PASS\n");
    return fails;
}

// ─────────────────────────────────────────────────────────────────────────────
//  TEST 3 – Top-level function givens_qr_top (full integration)
//
//  Uses the same diagonal L/Lt and unit steering vector as Test 2,
//  but feeds them through the AXI-interfaced top function.
//  Verifies the output matches Test 2's reference.
// ─────────────────────────────────────────────────────────────────────────────
static int test_top_function()
{
    printf("\n=== TEST 3: givens_qr_top (full AXI-interfaced top function) ===\n");

    static cplx_chol   Lt[M][M];
    static cplx_chol   L[M][M];
    static cplx_steer  a_s[M];
    static cplx_weight w_out[M];

    // Same diagonal setup as Test 2
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            Lt[i][j] = cplx_chol(t_chol(0), t_chol(0));
            L[i][j]  = cplx_chol(t_chol(0), t_chol(0));
        }
        double dv = 1.0 + i * 0.25;
        Lt[i][i] = cplx_chol(t_chol(dv), t_chol(0));
        L[i][i]  = cplx_chol(t_chol(dv), t_chol(0));
        a_s[i]   = cplx_steer(t_steer(i==0 ? 1.0 : 0.0), t_steer(0));
    }

    // Call top function
    givens_qr_top(Lt, L, a_s, w_out);

    // Reference: same as Test 2
    double w0_ref = 1.0 / (1.0 * 1.0);   // 1/(l_0)^2 = 1.0

    int fails = 0;
    double w0_re = (double)w_out[0].real();
    double w0_im = (double)w_out[0].imag();

    printf("  w_out[0] expected (%.5f, 0), got (%.5f, %.5f)\n",
           w0_ref, w0_re, w0_im);

    if (dabs(w0_re - w0_ref) > TOL_W || dabs(w0_im) > TOL_W) {
        printf("  FAIL w_out[0]: error re=%.2e im=%.2e\n",
               dabs(w0_re - w0_ref), dabs(w0_im));
        fails++;
    }

    for (int i = 1; i < M && fails <= 5; i++) {
        double wr = (double)w_out[i].real();
        double wi = (double)w_out[i].imag();
        if (dabs(wr) > TOL_W || dabs(wi) > TOL_W) {
            printf("  FAIL w_out[%d] = (%.5f, %.5f), expected (0, 0)\n", i, wr, wi);
            fails++;
        }
    }

    if (fails == 0) printf("  PASS\n");
    return fails;
}

// ─────────────────────────────────────────────────────────────────────────────
//  CORDIC self-test: verify cos/sin accuracy for known angles
// ─────────────────────────────────────────────────────────────────────────────
static int test_cordic()
{
    printf("\n=== TEST 0 (CORDIC sanity): cos/sin for known angles ===\n");

    struct { double angle; double exp_cos; double exp_sin; } cases[] = {
        {0.0,              1.0,  0.0 },
        {3.14159265/4.0,   0.7071067812, 0.7071067812 },
        {3.14159265/2.0,   0.0, 1.0 },
        {3.14159265/6.0,   0.8660254038, 0.5 },
        {-3.14159265/4.0,  0.7071067812, -0.7071067812 }
    };
    int n = sizeof(cases)/sizeof(cases[0]);
    int fails = 0;
    double cordic_tol = 5e-4;   // CORDIC 16-iter accuracy ≈ 1.5e-5 but fixed-point adds more

    for (int c = 0; c < n; c++) {
        t_cordic cos_out, sin_out;
        cordic_rotation(t_cordic(cases[c].angle), cos_out, sin_out);
        double got_cos = (double)cos_out;
        double got_sin = (double)sin_out;
        if (dabs(got_cos - cases[c].exp_cos) > cordic_tol ||
            dabs(got_sin - cases[c].exp_sin) > cordic_tol) {
            printf("  FAIL angle=%.4f: cos=(%.6f vs %.6f), sin=(%.6f vs %.6f)\n",
                   cases[c].angle, got_cos, cases[c].exp_cos,
                   got_sin, cases[c].exp_sin);
            fails++;
        }
    }
    if (fails == 0) printf("  PASS (%d angles tested, tol=%.1e)\n", n, cordic_tol);
    return fails;
}

// ─────────────────────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────────────────────
int main()
{
    printf("============================================================\n");
    printf(" Givens QR + MVDR Back-Sub – C-Simulation Testbench\n");
    printf(" M=%d  ITER=%d  TOL_QR=%.1e  TOL_W=%.1e\n",
           M, ITER, TOL_QR, TOL_W);
    printf("============================================================\n");

    int total = 0;
    total += test_cordic();
    total += test_qr_properties();
    total += test_back_substitution();
    total += test_top_function();

    printf("\n============================================================\n");
    if (total == 0)
        printf(" ALL TESTS PASSED\n");
    else
        printf(" FAILED: %d violation(s)\n", total);
    printf("============================================================\n");

    return total;
}
