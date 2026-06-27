// rdbs_cov_bridge.cpp
// =============================================================================
// PL Bridge Kernel — Gathers RDBS AIE output into full X_buf, broadcasts to
// the 4 Covariance AIE tiles.
// =============================================================================
// ⚠ THIS FILE IS THE LOAD-BEARING PIECE THAT MAKES THE AIE GRAPH IN graph.h
//   ACTUALLY WORK. Without it, "connect RDBS output directly to 4 Covariance
//   tile inputs" (as graph.h's connect<> calls literally describe) does NOT
//   implement the real data dependency, which is:
//
//     RDBS tile fires 1000 times (once per snapshot), each time producing
//     21 complex beams. Each Covariance tile needs the FULL 21x1000 buffer
//     -- not one snapshot at a time -- because computing R[i][j] requires
//     summing across ALL 1000 snapshots for channels i and j before it can
//     emit a single result.
//
//   adf::graph window connections move FIXED-SIZE windows per kernel
//   invocation; they do not have a built-in "accumulate N invocations into
//   one larger buffer" primitive. That accumulation has to happen
//   SOMEWHERE -- either inside an AIE kernel (consuming extra AIE tile
//   resources just for buffering) or in PL logic with a BRAM big enough to
//   hold 21 x 1000 x 4 bytes = 84,000 bytes. This file implements the PL
//   option because:
//     (a) it keeps the AIE Covariance tile's own local memory (already
//         tight at ~84KB, see covariance_aie.cc's header note) free of an
//         additional dedicated gather buffer,
//     (b) PL BRAM is comparatively abundant on VCK190 and well-suited to
//         this kind of bulk buffering + fan-out (a near-identical pattern
//         to your ORIGINAL covariance_matrix.cpp's X_buf design, which
//         this essentially reuses).
//
//   THIS FILE HAS NOT BEEN SYNTHESIZED OR SIMULATED. It is a structurally
//   sound first draft showing the intended PL<->AIE handshake, written in
//   Vitis HLS C++ syntax (to be added as a 4th PL kernel alongside MVDR
//   in your Vivado design), but you must run it through HLS C-simulation
//   before trusting timing or correctness.
// =============================================================================

#include <ap_int.h>
#include <hls_stream.h>

#define M_CH      21
#define K_SAMP    1000
#define N_TILES   4

// AXI-Stream packet matching the AIE PLIO output format: cint16 packed as
// 32-bit words (16-bit real, 16-bit imag), same convention as the original
// RDBS->Covariance AXI-Stream link in the PL-only design.
struct axis_pkt_t {
    ap_int<32> data;
    bool last;
};

void rdbs_cov_bridge(
    hls::stream<axis_pkt_t> &rdbs_in,        // from RDBS AIE tile via PLIO
    hls::stream<axis_pkt_t> &cov_out_tile0,  // to Covariance AIE tile 0
    hls::stream<axis_pkt_t> &cov_out_tile1,  // to Covariance AIE tile 1
    hls::stream<axis_pkt_t> &cov_out_tile2,  // to Covariance AIE tile 2
    hls::stream<axis_pkt_t> &cov_out_tile3)  // to Covariance AIE tile 3
{
#pragma HLS INTERFACE axis port=rdbs_in
#pragma HLS INTERFACE axis port=cov_out_tile0
#pragma HLS INTERFACE axis port=cov_out_tile1
#pragma HLS INTERFACE axis port=cov_out_tile2
#pragma HLS INTERFACE axis port=cov_out_tile3
#pragma HLS INTERFACE s_axilite port=return

    // Gather buffer: identical shape to the original covariance_matrix.cpp's
    // X_buf[M_CH][K_SAMP], partitioned the same way for the same reason
    // (rows i and j both readable same cycle during broadcast-out).
    static ap_int<32> X_gather[M_CH][K_SAMP];
#pragma HLS ARRAY_PARTITION variable=X_gather dim=1 complete

    // ── Gather phase: read 21*1000 packets from RDBS AIE output ─────────
    GATHER_SNAPSHOT:
    for (int t = 0; t < K_SAMP; t++) {
        GATHER_CHANNEL:
        for (int ch = 0; ch < M_CH; ch++) {
#pragma HLS PIPELINE II=1
            axis_pkt_t pkt = rdbs_in.read();
            X_gather[ch][t] = pkt.data;
        }
    }

    // ── Broadcast phase: send the FULL buffer to all 4 covariance tiles ──
    // Each tile receives the IDENTICAL 21x1000 buffer (any (i,j) pair might
    // reference any channel, so no subsetting is possible here -- this is
    // the broadcast cost flagged in covariance_aie.h's header comment).
    BROADCAST_CH:
    for (int ch = 0; ch < M_CH; ch++) {
        BROADCAST_T:
        for (int t = 0; t < K_SAMP; t++) {
#pragma HLS PIPELINE II=1
            axis_pkt_t out_pkt;
            out_pkt.data = X_gather[ch][t];
            out_pkt.last = (ch == M_CH - 1) && (t == K_SAMP - 1);

            cov_out_tile0.write(out_pkt);
            cov_out_tile1.write(out_pkt);
            cov_out_tile2.write(out_pkt);
            cov_out_tile3.write(out_pkt);
        }
    }
}
