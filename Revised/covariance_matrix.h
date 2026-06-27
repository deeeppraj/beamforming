// =============================================================================
//  covariance_matrix.h
//  Vitis HLS kernel – Sample Covariance Matrix Estimation
//  Target : Versal VCK190 Programmable Logic (PL)
//  Tool   : Vitis HLS 2024.1
//  Clock  : 300 MHz
//
//  Signal model
//    x(t) = sum_s  a(theta_s) * s_s(t)  +  n(t),   t = 0 .. K-1
//  Estimator
//    R_hat = (1/K) * X * X^H             [M x M, Hermitian PSD]
//
//  Array  : 8x8 URA → M_ANT = 64 antennas
//  Snapshots : K_SAMP = 2*M_ANT + 1 = 129
//
//  CHANGES FROM ORIGINAL:
//    [FIX-1]  result_t promoted from ap_fixed<32,8> to ap_fixed<32,8>  — KEPT
//             BUT typedef renamed to cov_result_t to make it unambiguous
//             at the integration boundary with givens_qr_top (which expects
//             ap_fixed<32,8> after widening of t_rxx — see givens_qr.h).
//    [FIX-2]  cresult_t renamed to ccov_result_t to avoid name collision
//             when both headers are included in a system testbench.
//    [FIX-3]  UTRI_ENTRIES corrected: M*(M+1)/2 = 2080 (was correct, confirmed).
//    [FIX-4]  Added K_INV_VAL constant for use in both kernel and testbench.
//    [FIX-5]  Added explicit function declaration for covariance_estimation.
// =============================================================================

#pragma once

#include <ap_fixed.h>
#include <ap_int.h>

// ─────────────────────────────────────────────────────────────────────────────
//  Array dimensions
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int M_ANT   = 64;                          // antenna elements
static constexpr int K_SAMP  = 129;                         // temporal snapshots
static constexpr int UTRI_ENTRIES = M_ANT*(M_ANT+1)/2;     // 2080

// Compile-time reciprocal – used in kernel AND testbench for golden reference
static constexpr double K_INV_VAL = 1.0 / K_SAMP;          // ≈ 0.007752

// ─────────────────────────────────────────────────────────────────────────────
//  Fixed-point type definitions
//
//  samp_t      : Input sample       – Q4.12  (16-bit, range ≈ ±8, res 2^-12)
//  accum_t     : Accumulator        – Q15.25 (40-bit)
//                Worst-case:  2 × 64 × 129 = 16 512  → 14 int bits + sign
//  cov_result_t: Output Rxx entry   – Q8.24  (32-bit)
//                After /K:   16512/129 = 128  → 8 int bits + sign
//
//  [FIX-1] result_t is now called cov_result_t to prevent ambiguity when
//  the system TB includes both this header and givens_qr.h.
// ─────────────────────────────────────────────────────────────────────────────
typedef ap_fixed<16, 4,  AP_RND_CONV, AP_SAT>  samp_t;
typedef ap_fixed<40, 15, AP_RND_CONV, AP_SAT>  accum_t;
typedef ap_fixed<32, 8,  AP_RND_CONV, AP_SAT>  cov_result_t;   // [FIX-1]

// ─── Complex structs ──────────────────────────────────────────────────────────
//  Plain structs: memory layout and bit-widths are explicit → synthesis
//  directives such as #pragma HLS DATA_PACK apply cleanly.
//  [FIX-2] Renamed cresult_t → ccov_result_t.

struct csamp_t {
    samp_t re;   // In-phase
    samp_t im;   // Quadrature
};

struct ccov_result_t {          // [FIX-2]
    cov_result_t re;
    cov_result_t im;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Top-level kernel declaration
//
//  Ports
//    X[M_ANT][K_SAMP]  – Input data matrix (antenna × time)
//    R[M_ANT][M_ANT]   – Output covariance matrix (Hermitian, complex)
// ─────────────────────────────────────────────────────────────────────────────
void covariance_estimation(
    csamp_t      X[M_ANT][K_SAMP],   // M × K input  (m_axi gmem0, read)
    ccov_result_t R[M_ANT][M_ANT]    // M × M output (m_axi gmem1, write)
);
