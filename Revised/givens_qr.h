#ifndef GIVENS_QR_H
#define GIVENS_QR_H

// ============================================================================
//  givens_qr.h — Header for MVDR Givens QR + Back-substitution IP
//  Target : Xilinx Versal VCK190  (xcvc1902-vsva2197-2MP-e-S)
//  Tool   : Vitis HLS 2024.1
//  Clock  : 300 MHz
//  Matrix : M×M complex fixed-point  (M=8 for dev, change to M=64 for final)
//
//  CHANGES FROM ORIGINAL:
//    [FIX-1]  t_rxx widened from ap_fixed<16,6> to ap_fixed<32,8> to match
//             the cov_result_t output of the covariance_estimation kernel.
//             This eliminates the integration boundary type mismatch.
//    [FIX-2]  cplx_rxx updated accordingly (hls::x_complex<t_rxx>).
//    [FIX-3]  t_accum integer bits raised from 7 to 9 to accommodate the
//             wider t_rxx (ap_fixed<32,8>) accumulation without overflow:
//             log2(M=64)=6 extra int bits, base int bits from t_rxx=8 → 14
//             total int bits needed; use 9 for M=8 dev, increase with M.
//    [FIX-4]  Added s_axilite port declarations (as comments) to remind that
//             ALL pointer arguments to givens_qr_top MUST have s_axilite
//             pragmas in the .cpp — they are the critical missing fix.
//    [FIX-5]  ATAN_TABLE changed from float[] to const t_cordic[] to avoid
//             implicit float→fixed conversion at each CORDIC iteration.
//             This removes one conversion multiplier per CORDIC step.
//    [FIX-6]  Added CORDIC_THRESHOLD_SQ constant for squared-magnitude
//             threshold — avoids hls::sqrt in the Givens inner loop.
// ============================================================================

#include "ap_fixed.h"
#include "hls_x_complex.h"
#include "ap_int.h"
#include "hls_math.h"

// ----------------------------------------------------------------------------
//  1. MATRIX DIMENSION
//     M=8 for development and validation.
//     Change to M=64 after M=8 passes C-sim AND co-simulation.
//     Also update t_accum integer bits when changing M (see FIX-3).
// ----------------------------------------------------------------------------
#define M       8       // array dimension (8 for dev, 64 for final)
#define ITER    16      // CORDIC iterations  (~1e-5 accuracy at 16 iterations)

// ----------------------------------------------------------------------------
//  2. FIXED-POINT BASE TYPES  (scalar real)
//     Format: ap_fixed<W, I>
//       W = total word length (bits)
//       I = integer bits (including sign bit)
//       Fractional bits = W - I
//     Rounding : AP_RND   (round to nearest — matches MATLAB 'Nearest')
//     Overflow  : AP_SAT  (saturate      — matches MATLAB 'Saturate')
// ----------------------------------------------------------------------------

// Input covariance matrix Rxx
// [FIX-1] Widened to ap_fixed<32,8> to match covariance_estimation output
//         (cov_result_t = ap_fixed<32,8>).  Max value after Rxx: ~128 → 8 int bits.
typedef ap_fixed<32, 8,  AP_RND, AP_SAT>   t_rxx;

// Cholesky factor L and Lt  (max ~11.3 for M=64: sqrt(128) → 4 int bits min;
//  use 5 for M=8, 5 for M=64 with room to spare)
typedef ap_fixed<16, 5,  AP_RND, AP_SAT>   t_chol;

// Q matrix  (unitary → all entries ≤ 1.0 in magnitude → 2 integer bits)
typedef ap_fixed<16, 2,  AP_RND, AP_SAT>   t_qmat;

// R matrix  (upper triangular; max ≈ sqrt(M)*max(Lt) → 18 bits, 5 int bits)
typedef ap_fixed<18, 5,  AP_RND, AP_SAT>   t_rmat;

// z and rhs vectors  (max ~0.48 → 1 integer bit)
typedef ap_fixed<16, 1,  AP_RND, AP_SAT>   t_zvec;

// Weight vector  (max ~0.023 → 1 integer bit, 17 fractional)
typedef ap_fixed<18, 1,  AP_RND, AP_SAT>   t_weight;

// Steering vector  (unit magnitude → 2 integer bits)
typedef ap_fixed<16, 2,  AP_RND, AP_SAT>   t_steer;

// CORDIC internal type — wide to accumulate without precision loss
// 32 bits: 4 integer + 28 fractional
typedef ap_fixed<32, 4,  AP_RND, AP_SAT>   t_cordic;

// Accumulator for dot products
// [FIX-3] For M=8:  max product ≈ 5.2 × 5.2 = 27; sum of 8 ≈ 216 → 9 int bits
//          For M=64: raise integer bits to 12 (sum of 64 × 27 ≈ 1728 → 11 bits)
typedef ap_fixed<34, 9,  AP_RND, AP_SAT>   t_accum;

// ----------------------------------------------------------------------------
//  3. COMPLEX TYPES (hls::x_complex<T>)
// ----------------------------------------------------------------------------
typedef hls::x_complex<t_rxx>    cplx_rxx;     // Rxx elements  [FIX-2]
typedef hls::x_complex<t_chol>   cplx_chol;    // L, Lt elements
typedef hls::x_complex<t_qmat>   cplx_qmat;    // Q elements
typedef hls::x_complex<t_rmat>   cplx_rmat;    // R elements
typedef hls::x_complex<t_zvec>   cplx_zvec;    // z, rhs elements
typedef hls::x_complex<t_weight> cplx_weight;  // weight vector
typedef hls::x_complex<t_steer>  cplx_steer;   // steering vector
typedef hls::x_complex<t_cordic> cplx_cordic;  // CORDIC working type

// ----------------------------------------------------------------------------
//  4. CORDIC CONSTANTS
//     K     = product_{i=0}^{ITER-1} sqrt(1 + 2^{-2i})  ≈ 1.6468  (ITER=16)
//     1/K   ≈ 0.6073  — used to compensate CORDIC gain
//
//  Pre-computed atan table: atan(2^{-i}) in radians, i = 0..ITER-1
//  [FIX-5] Stored as t_cordic (fixed-point) to avoid float→fixed conversion
//          at every CORDIC loop iteration.  Values are quantised to Q4.28
//          resolution (t_cordic = ap_fixed<32,4>).
// ----------------------------------------------------------------------------
#define CORDIC_K_INV    t_cordic(0.6073)   // 1/CORDIC_gain

// [FIX-6] Squared threshold for skipping CORDIC on near-zero elements.
//  Replaces  abs_rij > 1e-6  (which required hls::sqrt)
//  with      abs_rij_sq > THRESH_SQ  (pure multiply-and-compare, hardware-friendly).
//  Value: (1e-4)^2 = 1e-8 — looser than original 1e-6 to account for
//  fixed-point quantisation noise at the Q4.28 level.
#define CORDIC_THRESHOLD_SQ  t_cordic(1e-8)

// [FIX-5] atan table as t_cordic to avoid implicit float casts in the loop
static const t_cordic ATAN_TABLE[ITER] = {
    t_cordic(0.7853981634),   // atan(2^0)  = 45.000°
    t_cordic(0.4636476090),   // atan(2^-1) = 26.565°
    t_cordic(0.2449786631),   // atan(2^-2) = 14.036°
    t_cordic(0.1243549945),   // atan(2^-3) =  7.125°
    t_cordic(0.0624188100),   // atan(2^-4) =  3.576°
    t_cordic(0.0312398334),   // atan(2^-5) =  1.790°
    t_cordic(0.0156237286),   // atan(2^-6) =  0.895°
    t_cordic(0.0078123766),   // atan(2^-7) =  0.448°
    t_cordic(0.0039062301),   // atan(2^-8) =  0.224°
    t_cordic(0.0019531226),   // atan(2^-9) =  0.112°
    t_cordic(0.0009765622),   // atan(2^-10)=  0.056°
    t_cordic(0.0004882812),   // atan(2^-11)=  0.028°
    t_cordic(0.0002441406),   // atan(2^-12)=  0.014°
    t_cordic(0.0001220703),   // atan(2^-13)=  0.007°
    t_cordic(0.0000610352),   // atan(2^-14)=  0.0035°
    t_cordic(0.0000305176)    // atan(2^-15)=  0.0017°
};

// ----------------------------------------------------------------------------
//  5. FUNCTION DECLARATIONS
// ----------------------------------------------------------------------------

// CORDIC vectoring mode: (x_in, y_in) → magnitude, angle=atan2(y_in, x_in)
void cordic_vectoring(t_cordic  x_in,
                      t_cordic  y_in,
                      t_cordic &magnitude,
                      t_cordic &angle);

// CORDIC rotation mode: angle → cos(angle), sin(angle)
void cordic_rotation(t_cordic  angle,
                     t_cordic &cos_out,
                     t_cordic &sin_out);

// Givens QR: A[M][M] → Q[M][M] (unitary), R[M][M] (upper triangular)
//   A = Q * R
void givens_qr(cplx_chol A[M][M],
               cplx_qmat Q[M][M],
               cplx_rmat R[M][M]);

// Back-substitution: solves Rxx·w = a_s via QR of Lt
//   Steps: (1) L·z = a_s  (forward sub)
//          (2) rhs = Q^H · z
//          (3) R·w = rhs  (back sub)
void back_substitution(cplx_qmat   Q[M][M],
                       cplx_rmat   R[M][M],
                       cplx_chol   L[M][M],
                       cplx_steer  a_s[M],
                       cplx_weight w[M]);

// TOP FUNCTION — synthesised into hardware IP
//   [FIX-4] All pointer ports MUST also have s_axilite pragmas in the .cpp
//           (see givens_qr.cpp) so the PS can write their DDR base addresses.
void givens_qr_top(cplx_chol   Lt[M][M],    // input: upper Cholesky factor
                   cplx_chol   L[M][M],     // input: lower Cholesky factor
                   cplx_steer  a_s[M],      // input: steering vector
                   cplx_weight w_out[M]);   // output: unnormalised weight vector

#endif // GIVENS_QR_H
