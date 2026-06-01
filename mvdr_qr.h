#pragma once
// ============================================================
//  mvdr_qr.h  –  QR-MVDR Beamformer HLS Header
//  Target : Versal AI Core VCK190 / VCK5000
//  Tool   : Vitis HLS 2024.1
// ============================================================
#include <ap_fixed.h>
#include <hls_math.h>
#include <complex>

// ─── Array / snapshot dimensions ────────────────────────────
#define M        8     // number of antenna elements
#define N_SNAP   64    // snapshots  (must be >= M)
#define M_PAD    8     // padded to power-of-2 (same here)

// ─── Fixed-point typedefs ───────────────────────────────────
// Input data : Q1.15  (range [-1, 1))
typedef ap_fixed<16, 2>  data_t;
// Accumulator : Q8.24  (avoids overflow in R = X*X^H)
typedef ap_fixed<32, 8>  acc_t;
// Weight output: Q2.14
typedef ap_fixed<16, 2>  weight_t;

// Complex wrappers
typedef std::complex<data_t>   cdata_t;
typedef std::complex<acc_t>    cacc_t;
typedef std::complex<weight_t> cweight_t;

// ─── Top-level function ──────────────────────────────────────
// X      : M x N_SNAP received array data  (column-major flat)
// a_s    : M x 1 steering vector
// w_out  : M x 1 MVDR weights output
void mvdr_qr_top(
    cdata_t   X[M * N_SNAP],   // input  – flattened column-major
    cdata_t   a_s[M],          // input  – steering vector
    cweight_t w_out[M]         // output – beamforming weights
);

// ─── Sub-module declarations ─────────────────────────────────
void compute_R(
    cdata_t X[M * N_SNAP],
    cacc_t  R[M * M]
);

void householder_qr(
    cacc_t  A[M * M],          // in-place: overwritten with R
    cacc_t  Q[M * M]           // optional Q (can be ignored for MVDR)
);

void forward_substitution(
    cacc_t  L[M * M],          // lower-triangular (R^H)
    cacc_t  b[M],
    cacc_t  y[M]
);

void back_substitution(
    cacc_t  U[M * M],          // upper-triangular (R)
    cacc_t  y[M],
    cacc_t  z[M]
);

void normalize_weight(
    cacc_t    z[M],
    cdata_t   a_s[M],
    cweight_t w_out[M]
);
