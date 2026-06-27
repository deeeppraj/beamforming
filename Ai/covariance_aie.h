// covariance_aie.h
// =============================================================================
// Sample Covariance Estimation — AI Engine Kernel(s)
// =============================================================================
// AIE re-implementation of covariance_matrix.cpp's PHASE 2:
//   R[i][j] = (1/K) * sum_t X[i][t] * conj(X[j][t])     for all (i,j) pairs
//
// ⚠ READ THIS SECTION CAREFULLY — this is the part of the design that has
//   NOT been hardware/simulator validated. The partitioning strategy below
//   is a reasonable starting point, not a verified-correct topology. You
//   MUST run this through the AIE single-tile and multi-tile simulator
//   before trusting timing or correctness numbers from it.
//
// PARTITIONING STRATEGY
// ─────────────────────
// There are 231 independent (i,j) upper-triangle pairs (i <= j, 21x21).
// Each pair needs 1000 complex MACs (one per snapshot) — completely
// independent of every other pair, which is exactly the kind of
// parallelism AIE tiles are good at splitting across multiple tiles.
//
// This design uses 4 AIE tiles, each computing a contiguous block of
// ~58 (i,j) pairs (231 / 4 ≈ 57.75). The blocks are NOT equal-sized in
// an i-major triangular split (row 0 has 21 pairs, row 20 has 1 pair),
// so a naive "give each tile 5-6 rows" split creates a severe load
// imbalance: tile 0 (rows 0-4) would get 21+20+19+18+17 = 95 pairs,
// while tile 3 (rows 15-20) gets only 6+5+4+3+2+1 = 21 pairs — a 4.5x
// imbalance that would make 3 tiles idle while tile 0 is still working.
//
// INSTEAD, this design pre-computes a flattened list of all 231 (i,j)
// indices on the HOST/PL side (at compile time, via the PAIR_LIST table
// below) and assigns pairs to tiles round-robin (pair_idx % 4), which
// gives each tile ~58 pairs of roughly similar total work since MAC count
// per pair is constant (1000) regardless of which (i,j) it is. This is
// the standard fix for triangular-matrix load imbalance and is WHY this
// header pre-generates PAIR_LIST_TILE0..3 instead of using nested i/j
// loops directly inside each kernel.
//
// EACH TILE receives the FULL 21-channel, 1000-snapshot X buffer (not a
// subset) because any (i,j) pair might reference any channel. This means
// X_buf (21 x 1000 x cint16 = 21000 x 4 bytes = 84,000 bytes) must be
// broadcast to all 4 tiles. AIE-ML tile local memory is typically
// 64KB/tile, so 84KB does NOT fit in one tile's local memory alone —
// this is the single biggest open risk in this design (see note at the
// bottom of this file).
// =============================================================================
#ifndef COVARIANCE_AIE_H
#define COVARIANCE_AIE_H

#include <adf.h>
#include <aie_api/aie_adf.hpp>

#define M_CH       21
#define K_SAMP     1000
#define UTRI_TOTAL 231          // 21*22/2

#define N_COV_TILES 4
// Pairs per tile: ceil(231/4) = 58 for tiles 0-2, 57 for tile 3 (231 = 58*3+57)
#define PAIRS_TILE_0 58
#define PAIRS_TILE_1 58
#define PAIRS_TILE_2 58
#define PAIRS_TILE_3 57

// ─── Fixed-point shift (same logic as RDBS) ─────────────────────────────────
// samp_t = ap_fixed<16,4>  -> 12 frac bits  (this is the INPUT to covariance,
//                                            i.e. RDBS's OUTPUT format)
// result_t = ap_fixed<32,9> -> 23 frac bits (covariance's OUTPUT format)
//
// Product of two Q12 numbers -> Q24 in the 48-bit accumulator.
// Then divide by K (=1000, NOT a power of 2 — see note below) and
// rescale Q24 -> Q23 (shift right by 1) to land on result_t's format.
//
// ⚠ K_SAMP = 1000 is NOT a power of two, so unlike RDBS's clean bit-shift
//   normalization, the (1/K) division here cannot be a pure right-shift.
//   The PL version handles this with accum_t(1.0 / K_SAMP) — a true
//   fixed-point multiply by a precomputed reciprocal constant, not a
//   shift. The AIE version MUST do the same: multiply by the constant
//   INV_K_Q15 (1/1000 represented in Q15 fixed point) using a SECOND
//   multiply pass, rather than trying to fold it into the shift amount.
//   This is implemented in covariance_aie.cc as an explicit scalar
//   multiply after the MAC accumulation, not as part of COV_SHIFT.
#define COV_SHIFT     1                    // Q24 -> Q23 after K-division
#define INV_K_Q15     33                   // round(1/1000 * 2^15) = 32.768 -> 33

// ─── Output packing ──────────────────────────────────────────────────────
// Each tile produces its assigned pairs as (re, im) cint32 results.
// Output window per tile = PAIRS_TILE_n * 1 cint32 element (one complex
// result per pair). The PL-side gather kernel (or PL logic) reassembles
// these into the full 21x21 R matrix using the same PAIR_LIST ordering.
struct pair_idx_t {
    unsigned char i;
    unsigned char j;
};

// Tile 0: pairs 0..57 of the flattened upper-triangle enumeration
// (flattened order: (0,0),(0,1),...,(0,20),(1,1),(1,2),...,(20,20))
extern const pair_idx_t PAIR_LIST_TILE0[PAIRS_TILE_0];
extern const pair_idx_t PAIR_LIST_TILE1[PAIRS_TILE_1];
extern const pair_idx_t PAIR_LIST_TILE2[PAIRS_TILE_2];
extern const pair_idx_t PAIR_LIST_TILE3[PAIRS_TILE_3];

// ─── Per-tile kernel function (same code, different PAIR_LIST + window) ────
// X_buf_window: ALL 21 channels x 1000 snapshots, broadcast to every tile.
// pair_list:    this tile's assigned (i,j) indices (compile-time array).
// n_pairs:      how many pairs this tile handles (58 or 57).
// r_out:        cint32 result per pair, length n_pairs.
void covariance_tile_kernel(
    input_window_cint16  * __restrict X_buf_window,  // 21*1000 cint16 = 84,000B
    const pair_idx_t     * __restrict pair_list,
    unsigned                          n_pairs,
    output_window_cint32 * __restrict r_out
);

#endif // COVARIANCE_AIE_H
