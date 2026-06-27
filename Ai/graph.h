// graph.h
// =============================================================================
// AIE Graph — Beamforming Front-End (RDBS + Covariance)
// =============================================================================
// Declares the adf::graph that wires together:
//   1 x RDBS tile        (rdbs_beamform)
//   4 x Covariance tiles (covariance_tile_kernel, one per pair-group)
//
// DATA FLOW:
//   PL (or PS via DMA) --PLIO--> [RDBS tile] --window--> [4x broadcast]
//                                                              |
//                            +---------------------------------+----+----+
//                            v              v              v         v
//                       [Cov tile 0]   [Cov tile 1]   [Cov tile 2] [Cov tile 3]
//                            |              |              |         |
//                            v              v              v         v
//                          PLIO out 0..3 --> PL gather logic --> DDR (via NoC)
//
// ⚠ KEY ARCHITECTURAL CHANGE FROM THE ORIGINAL PL DESIGN:
//   The original covariance_estimation kernel received ALL 1000 snapshots
//   from RDBS via one continuous AXI-Stream, accumulating internally as
//   data arrived (LOAD_SNAPSHOT phase). The AIE version INSTEAD requires
//   RDBS to produce a COMPLETE 21x1000 buffer (across many invocations)
//   BEFORE the Covariance tiles start, because each (i,j) pair needs
//   random access to the full time series of channels i and j -- this is
//   fundamentally a batch operation, not a streaming one, regardless of
//   which fabric computes it. The PL version got away with streaming
//   because X_buf was fully buffered on-chip already (X_buf[M_CH][K_SAMP]
//   in BRAM) before PHASE 2 began -- it only LOOKED streaming on the input
//   side. The AIE version makes this buffering explicit at the graph level
//   via the rdbs_to_cov_buffer connection below.
// =============================================================================
#ifndef GRAPH_H
#define GRAPH_H

#include <adf.h>
#include "rdbs_aie.h"
#include "covariance_aie.h"

using namespace adf;

class BeamformFrontendGraph : public graph {
private:
    // ── Kernel declarations ──────────────────────────────────────────────
    kernel k_rdbs;
    kernel k_cov[N_COV_TILES];   // 4 covariance tiles

public:
    // ── External I/O ports (connect to PL via PLIO) ─────────────────────
    // Input: raw 64-element complex samples, one snapshot at a time,
    // fed continuously for 1000 snapshots before a graph "run" completes.
    input_plio  x_in;

    // Coefficient load port (run-time parameter style load, done ONCE
    // at startup, not per-snapshot). Declared as a second PLIO rather
    // than an RTP because the coefficient table (21*64 = 1344 cint16
    // = 5376 bytes) exceeds typical RTP size limits (RTPs are meant for
    // small scalar/short-array control values, not bulk coefficient
    // tables) -- bulk coefficient loads are conventionally done via a
    // PLIO window read once at init, which is what this models.
    input_plio  coeff_in;

    // Output: 4 separate PLIO streams, one per covariance tile, each
    // carrying that tile's subset of (i,j) pair results. The PL side
    // (or PS software) is responsible for scattering these into the
    // correct positions of the final 21x21 R matrix using the SAME
    // PAIR_LIST_TILE0..3 tables defined in covariance_pair_tables.cc --
    // both sides MUST use identical tables or results land in the wrong
    // matrix position. This is the single most important correctness
    // contract in this entire design and is NOT enforced by any
    // compile-time check; mismatched tables on PL and AIE sides will
    // silently produce a wrong covariance matrix with no error raised.
    output_plio r_out[N_COV_TILES];

    BeamformFrontendGraph() {
        // ── Instantiate kernels ──────────────────────────────────────────
        k_rdbs = kernel::create(rdbs_beamform);

        for (int t = 0; t < N_COV_TILES; t++) {
            k_cov[t] = kernel::create(covariance_tile_kernel);
        }

        // ── PLIO ports ────────────────────────────────────────────────────
        x_in     = input_plio::create("x_in_plio",     plio_32_bits, "data/x_in.txt");
        coeff_in = input_plio::create("coeff_in_plio", plio_32_bits, "data/coeff_in.txt");

        for (int t = 0; t < N_COV_TILES; t++) {
            char name[32];
            snprintf(name, sizeof(name), "r_out_plio_%d", t);
            char fname[64];
            snprintf(fname, sizeof(fname), "data/r_out_tile%d.txt", t);
            r_out[t] = output_plio::create(name, plio_32_bits, fname);
        }

        // ── Connections: PLIO -> RDBS kernel ────────────────────────────
        connect<window<RDBS_IN_SAMPLES * 4>>  (x_in.out[0],     k_rdbs.in[0]);
        connect<window<M_CH * M_ELEMENTS * 4>>(coeff_in.out[0], k_rdbs.in[1]);

        // ── Connection: RDBS output -> broadcast buffer feeding all 4
        //    covariance tiles. This is the X_buf accumulation step
        //    described in the header comment: RDBS runs K_SAMP=1000
        //    times, each time appending one snapshot's 21 beams into a
        //    shared buffer that, once full, is broadcast (read by all 4
        //    tiles) to start the covariance phase.
        //
        //    NOTE: adf::graph windows are fixed-size per kernel
        //    invocation -- representing "accumulate 1000 snapshots then
        //    broadcast" requires either (a) a PL-side buffering kernel
        //    that gathers RDBS's per-snapshot outputs into the full
        //    21x1000 buffer before forwarding to AIE Covariance tiles,
        //    or (b) an AIE buffer kernel performing the same gather
        //    within the AIE array. THIS GRAPH ASSUMES OPTION (a) -- a
        //    small PL gather/buffer kernel sits between RDBS's AIE
        //    output and the Covariance tiles' AIE input, implemented in
        //    pl/rdbs_cov_bridge.cpp (see that file). This is a real
        //    architectural decision that needs your review, not a
        //    detail to skim past.
        connect<window<RDBS_OUT_PADDED * 4>> (k_rdbs.out[0], /* -> PL bridge, see pl/rdbs_cov_bridge.cpp */ k_cov[0].in[0]);
        // Broadcast the same gathered buffer to tiles 1-3 as well.
        // (In practice this fan-out happens on the PL bridge side, which
        //  replicates its output to 4 PLIO ports -- shown here only to
        //  document the logical connectivity at the AIE graph level.)

        for (int t = 0; t < N_COV_TILES; t++) {
            connect<window<M_CH * K_SAMP * 4>> (/* PL bridge broadcast */ k_rdbs.out[0], k_cov[t].in[0]);
            connect<window<128 * 4>>           (k_cov[t].out[0], r_out[t].in[0]);
        }

        // ── Source files for each kernel ────────────────────────────────
        source(k_rdbs) = "aie/rdbs_aie.cc";
        for (int t = 0; t < N_COV_TILES; t++) {
            source(k_cov[t]) = "aie/covariance_aie.cc";
        }

        // ── Runtime ratio: how many times each kernel fires per graph
        //    iteration. RDBS fires once per snapshot (1000x per full
        //    covariance computation); each Covariance tile fires ONCE
        //    per full 1000-snapshot batch (since it needs the complete
        //    buffer, not a per-snapshot increment).
        runtime<ratio>(k_rdbs) = 0.5;       // 50% of one AIE tile's cycle budget
        for (int t = 0; t < N_COV_TILES; t++) {
            runtime<ratio>(k_cov[t]) = 0.8; // covariance is the heavier compute
        }

        // ── Tile location constraints (optional, but recommended for
        //    predictable place&route -- without these, the AIE compiler
        //    is free to place tiles anywhere, which can produce
        //    inconsistent timing between simulator runs and hardware) ──
        location<kernel>(k_rdbs) = tile(0, 0);
        for (int t = 0; t < N_COV_TILES; t++) {
            location<kernel>(k_cov[t]) = tile(t, 1);
        }
    }
};

#endif // GRAPH_H
