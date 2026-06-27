#ifndef GIVENS_QR_H
#define GIVENS_QR_H

// ============================================================================
// givens_qr.h  —  Header file for MVDR Givens QR + Back-substitution
// Target  : Xilinx Versal VCK190  (xcvc1902-vsva2197-2MP-e-S)
// Tool    : Vitis HLS 2024.1
// Clock   : 300 MHz
// Matrix  : M×M complex fixed-point  (start with M=8, scale to M=64)
//
// Fixed-point word lengths confirmed from MATLAB fixed-point validation:
//   Rxx    : ap_fixed<16, 6>   max~27.5
//   L/Lt   : ap_fixed<16, 4>   max~5.2
//   Q      : ap_fixed<16, 2>   max~1.0  (unitary)
//   R      : ap_fixed<18, 4>   max~5.2  + guard bits
//   z/rhs  : ap_fixed<16, 1>   max~0.48
//   weights: ap_fixed<18, 1>   max~0.023
//   steer  : ap_fixed<16, 2>   max~1.0  (unit magnitude)
// ============================================================================

#include "ap_fixed.h"           // Xilinx arbitrary precision fixed-point
#include "hls_x_complex.h"      // Xilinx complex number type for HLS
#include "ap_int.h"             // Xilinx arbitrary precision integer
#include "hls_math.h"           // HLS math: hls::sqrt, hls::atan2 etc.

// ----------------------------------------------------------------------------
// 1. MATRIX DIMENSION
//    Start with M=8 for development and validation.
//    Change to M=64 only after M=8 passes C-sim and co-simulation.
// ----------------------------------------------------------------------------
#define M       8               // array dimension (8 for dev, 64 for final)
#define ITER    16              // CORDIC iterations (16 gives ~1e-5 accuracy)

// ----------------------------------------------------------------------------
// 2. FIXED-POINT BASE TYPES  (scalar real)
//    Format: ap_fixed<W, I>
//      W = total word length (bits)
//      I = integer bits (including sign)
//      Fractional bits = W - I
//    Rounding : AP_RND   (round to nearest — matches MATLAB fimath 'Nearest')
//    Overflow  : AP_SAT  (saturate    — matches MATLAB fimath 'Saturate')
// ----------------------------------------------------------------------------

// Input covariance matrix Rxx  (max ~27.5 → needs 6 integer bits)
typedef ap_fixed<16, 6,  AP_RND, AP_SAT>   t_rxx;

// Cholesky factor L and Lt  (max ~5.2 → needs 4 integer bits)
typedef ap_fixed<16, 4,  AP_RND, AP_SAT>   t_chol;

// Q matrix  (unitary → all entries ≤ 1.0 → 2 integer bits sufficient)
typedef ap_fixed<16, 2,  AP_RND, AP_SAT>   t_qmat;

// R matrix  (upper triangular, max ~5.2, +2 guard bits → 18 bits)
typedef ap_fixed<18, 4,  AP_RND, AP_SAT>   t_rmat;

// z and rhs vectors  (max ~0.48 → 1 integer bit)
typedef ap_fixed<16, 1,  AP_RND, AP_SAT>   t_zvec;

// Weight vectors w_unnorm and W_MVDR  (max ~0.023 → 1 integer bit, 17 frac)
typedef ap_fixed<18, 1,  AP_RND, AP_SAT>   t_weight;

// Steering vector a_s  (unit magnitude → 2 integer bits)
typedef ap_fixed<16, 2,  AP_RND, AP_SAT>   t_steer;

// CORDIC internal type  — wider to accumulate without precision loss
// 32 bits: 4 integer + 28 fractional
typedef ap_fixed<32, 4,  AP_RND, AP_SAT>   t_cordic;

// Accumulator type for dot products  (sum of M products → needs log2(M) extra
// integer bits to avoid overflow: log2(8)=3 extra, log2(64)=6 extra)
typedef ap_fixed<34, 7,  AP_RND, AP_SAT>   t_accum;

// ----------------------------------------------------------------------------
// 3. COMPLEX TYPES
//    hls::x_complex<T> provides .real() and .imag() accessors,
//    and overloads +, -, *, conj() for fixed-point arithmetic.
// ----------------------------------------------------------------------------
typedef hls::x_complex<t_rxx>    cplx_rxx;    // Rxx elements
typedef hls::x_complex<t_chol>   cplx_chol;   // L, Lt elements
typedef hls::x_complex<t_qmat>   cplx_qmat;   // Q elements
typedef hls::x_complex<t_rmat>   cplx_rmat;   // R elements
typedef hls::x_complex<t_zvec>   cplx_zvec;   // z, rhs elements
typedef hls::x_complex<t_weight> cplx_weight; // weight vector elements
typedef hls::x_complex<t_steer>  cplx_steer;  // steering vector elements
typedef hls::x_complex<t_cordic> cplx_cordic; // CORDIC working type

// ----------------------------------------------------------------------------
// 4. CORDIC CONSTANTS
//    CORDIC gain K = product of cos(atan(2^-i)) for i=0..ITER-1
//    K ≈ 1.6468  for ITER=16
//    We store 1/K so we can multiply instead of divide for compensation.
//    Computed offline: 1/1.6468 ≈ 0.6073
// ----------------------------------------------------------------------------
#define CORDIC_K_INV    0.6073  // 1 / CORDIC_gain  — used to compensate gain

// Pre-computed atan table: atan(2^-i) for i = 0, 1, ..., ITER-1
// Units: radians, stored as t_cordic fixed-point
// atan(2^0)  = 0.7854 rad  (45°)
// atan(2^-1) = 0.4636 rad
// atan(2^-2) = 0.2450 rad  ...
static const float ATAN_TABLE[ITER] = {
    0.7853981634f,  // atan(2^0)
    0.4636476090f,  // atan(2^-1)
    0.2449786631f,  // atan(2^-2)
    0.1243549945f,  // atan(2^-3)
    0.0624188100f,  // atan(2^-4)
    0.0312398334f,  // atan(2^-5)
    0.0156237286f,  // atan(2^-6)
    0.0078123766f,  // atan(2^-7)
    0.0039062301f,  // atan(2^-8)
    0.0019531226f,  // atan(2^-9)
    0.0009765622f,  // atan(2^-10)
    0.0004882812f,  // atan(2^-11)
    0.0002441406f,  // atan(2^-12)
    0.0001220703f,  // atan(2^-13)
    0.0000610352f,  // atan(2^-14)
    0.0000305176f   // atan(2^-15)
};

// ----------------------------------------------------------------------------
// 5. FUNCTION DECLARATIONS
//    These are the functions implemented in givens_qr.cpp
// ----------------------------------------------------------------------------

// CORDIC vectoring mode:
//   Input : (x_in, y_in) — real scalars
//   Output: magnitude = sqrt(x_in² + y_in²) × CORDIC_K_INV (compensated)
//           angle     = atan2(y_in, x_in) in radians
void cordic_vectoring(t_cordic  x_in,
                      t_cordic  y_in,
                      t_cordic &magnitude,
                      t_cordic &angle);

// CORDIC rotation mode:
//   Input : angle in radians
//   Output: cos(angle), sin(angle)
//   Used to get cs and sn from the angle computed by vectoring mode
void cordic_rotation(t_cordic  angle,
                     t_cordic &cos_out,
                     t_cordic &sin_out);

// Givens QR decomposition of an M×M complex matrix A
//   Input : A[M][M]  — upper triangular Cholesky factor Lt
//   Output: Q[M][M]  — unitary matrix
//           R[M][M]  — upper triangular matrix
//   Such that A = Q * R
void givens_qr(cplx_chol A[M][M],
               cplx_qmat Q[M][M],
               cplx_rmat R[M][M]);

// Back-substitution: solves for MVDR weights
//   Input : Q[M][M], R[M][M] from givens_qr
//           a_s[M]            — steering vector
//           L[M][M]           — Cholesky factor (lower triangular)
//   Output: w[M]              — unnormalised weight vector
//   Solves: L·L'·w = a_s  via QR factors of L'
void back_substitution(cplx_qmat   Q[M][M],
                       cplx_rmat   R[M][M],
                       cplx_chol   L[M][M],
                       cplx_steer  a_s[M],
                       cplx_weight w[M]);

// TOP FUNCTION — this is what Vitis HLS synthesises into hardware
//   AXI4-Stream interfaces for Lt and a_s input, w output
//   AXI4-Lite interface for control (start, done flags)
void givens_qr_top(cplx_chol   Lt[M][M],   // input: upper Cholesky factor
                   cplx_chol   L[M][M],    // input: lower Cholesky factor
                   cplx_steer  a_s[M],     // input: steering vector
                   cplx_weight w_out[M]);  // output: weight vector

#endif // GIVENS_QR_H
