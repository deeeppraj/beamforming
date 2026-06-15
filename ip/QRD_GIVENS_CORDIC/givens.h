#ifndef GIVENS_QR_H
#define GIVENS_QR_H
// ============================================================================
// givens_qr.h — Header for MVDR Givens QR + Back-substitution IP
// Target : Xilinx Versal VCK190 (xcvc1902-vsva2197-2MP-e-S)
// Tool   : Vitis HLS 2024.1
// Clock  : 300 MHz
//
// FIXES APPLIED IN THIS VERSION:
// [FIX-1] t_cordic widened from ap_fixed<32,4> to ap_fixed<32,6>:
//         Range ±8 → ±32, preventing saturation when Cholesky L diagonal
//         values (up to ~11.3 at M=64) are cast into the cplx_cordic
//         workspace arrays Rw/Qw inside givens_qr().
//
// [FIX-2] t_accum widened from ap_fixed<37,12> to ap_fixed<40,14>:
//         Worst-case: |L[j][k]|^2 ≤ (11.3)^2 ≈ 128, summed over 63
//         terms → max ~8064, requiring ceil(log2(8064))+1 = 14 integer bits.
//         Previous 12 bits (range ±4096) would overflow real Rxx data.
//
// [FIX-3] hls::sqrt() now operates directly on ap_fixed types (no float
//         cast). A negative-argument guard (clamp to zero) is added to
//         prevent NaN on non-PD Rxx input. See cholesky_core() in givens.cpp.
// ============================================================================

#include "ap_fixed.h"
#include "hls_x_complex.h"
#include "ap_int.h"
#include "hls_math.h"

// ----------------------------------------------------------------------------
// 1. MATRIX DIMENSION
// ----------------------------------------------------------------------------
#define M    64      // Scaled to 64 for final integration
#define ITER 16      // CORDIC iterations

// ----------------------------------------------------------------------------
// 2. FIXED-POINT BASE TYPES
// ----------------------------------------------------------------------------

// Input covariance matrix Rxx
// AP_RND_CONV matches upstream covariance IP (cov_result_t boundary)
typedef ap_fixed<32, 8, AP_RND_CONV, AP_SAT> t_rxx;

// Cholesky factor L and Lt
typedef ap_fixed<16, 5, AP_RND, AP_SAT>      t_chol;

// Q matrix (unitary, bounded ±1)
typedef ap_fixed<16, 2, AP_RND, AP_SAT>      t_qmat;

// R matrix (upper triangular)
typedef ap_fixed<18, 5, AP_RND, AP_SAT>      t_rmat;

// z and rhs vectors
typedef ap_fixed<16, 1, AP_RND, AP_SAT>      t_zvec;

// Weight vector output
typedef ap_fixed<18, 1, AP_RND, AP_SAT>      t_weight;

// Steering vector input
typedef ap_fixed<16, 2, AP_RND, AP_SAT>      t_steer;

// [FIX-1] CORDIC internal working type.
// Previous: ap_fixed<32,4>  → range ±8.  Saturated for L_diag up to ~11.3.
// Fixed:    ap_fixed<32,6>  → range ±32. Covers worst-case L_diag with margin.
// The extra 2 integer bits cost nothing in the fraction (total width unchanged),
// and CORDIC angle accumulation stays well within ±π < ±4 < ±32.
typedef ap_fixed<32, 6, AP_RND, AP_SAT>      t_cordic;

// [FIX-2] Accumulator for dot products in cholesky_core.
// Previous: ap_fixed<37,12> → max accumulation ±4096. Overflows at M=64
//           where sum of |L[j][k]|^2 can reach ~8064.
// Fixed:    ap_fixed<40,14> → range ±8192. Covers worst-case with margin.
// Fraction bits: 40-14 = 26 (was 37-12 = 25). Minimal precision impact.
typedef ap_fixed<40, 14, AP_RND, AP_SAT>     t_accum;

// ----------------------------------------------------------------------------
// 3. COMPLEX TYPES (hls::x_complex<T>)
// ----------------------------------------------------------------------------
typedef hls::x_complex<t_rxx>    cplx_rxx;
typedef hls::x_complex<t_chol>   cplx_chol;
typedef hls::x_complex<t_qmat>   cplx_qmat;
typedef hls::x_complex<t_rmat>   cplx_rmat;
typedef hls::x_complex<t_zvec>   cplx_zvec;
typedef hls::x_complex<t_weight> cplx_weight;
typedef hls::x_complex<t_steer>  cplx_steer;
typedef hls::x_complex<t_cordic> cplx_cordic;
typedef hls::x_complex<t_accum>  cplx_accum;  // Cholesky off-diagonal accumulator

// ----------------------------------------------------------------------------
// 4. CORDIC CONSTANTS
// ----------------------------------------------------------------------------
// K_INV = product_{i=0}^{ITER-1} cos(atan(2^{-i})) ≈ 0.6073
#define CORDIC_K_INV        t_cordic(0.6073)
#define CORDIC_THRESHOLD_SQ t_cordic(1e-8)

static const t_cordic ATAN_TABLE[ITER] = {
    t_cordic(0.7853981634), t_cordic(0.4636476090), t_cordic(0.2449786631),
    t_cordic(0.1243549945), t_cordic(0.0624188100), t_cordic(0.0312398334),
    t_cordic(0.0156237286), t_cordic(0.0078123766), t_cordic(0.0039062301),
    t_cordic(0.0019531226), t_cordic(0.0009765622), t_cordic(0.0004882812),
    t_cordic(0.0002441406), t_cordic(0.0001220703), t_cordic(0.0000610352),
    t_cordic(0.0000305176)
};

// ----------------------------------------------------------------------------
// 5. FUNCTION DECLARATIONS
// ----------------------------------------------------------------------------
void cordic_vectoring(t_cordic x_in, t_cordic y_in,
                      t_cordic &magnitude, t_cordic &angle);
void cordic_rotation(t_cordic angle, t_cordic &cos_out, t_cordic &sin_out);

// Integrated Cholesky Core: Rxx -> L (lower triangular) + Lt (conjugate transpose)
void cholesky_core(cplx_rxx Rxx_in[M][M],
                   cplx_chol L_out[M][M],
                   cplx_chol Lt_out[M][M]);

void givens_qr(cplx_chol A[M][M], cplx_qmat Q[M][M], cplx_rmat R[M][M]);

void back_substitution(cplx_qmat Q[M][M], cplx_rmat R[M][M], cplx_chol L[M][M],
                       cplx_steer a_s[M], cplx_weight w[M]);

// Top-level kernel: accepts Rxx, steering vector; outputs MVDR weights
void givens_qr_top(cplx_rxx    Rxx[M][M],
                   cplx_steer  a_s[M],
                   cplx_weight w_out[M]);

#endif // GIVENS_QR_H