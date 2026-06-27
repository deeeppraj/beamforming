// =============================================================================
//  covariance_matrix.cpp
//  Vitis HLS – Sample Covariance Matrix Kernel for Versal VCK190 PL
//
//  Algorithm
//    Phase 1 – Burst-load X from DDR into on-chip BRAM (X_buf).
//    Phase 2 – For every (i,j) pair in the upper triangle:
//                acc  +=  X[i][t] * conj(X[j][t])   for t = 0..K-1
//                R_buf[i][j] = acc / K
//                R_buf[j][i] = conj(R_buf[i][j])    (Hermitian fill, free)
//    Phase 3 – Burst-write R_buf to R via AXI4 in one contiguous burst.
//
//  HLS strategy
//    • X_buf  partitioned complete on dim=1 (64 independent BRAMs, one per
//      antenna row) so rows i and j are always readable in the same cycle.
//    • R_buf  partitioned complete on dim=1 (64 BRAMs, one per output row);
//      writes to R_buf[i][j] and R_buf[j][i] go to different physical banks.
//    • Inner K-loop pipelined II=1; each iteration: one cmac_conj = 4 DSPs.
//    • Normalization by 1/K: compile-time fixed-point constant → 1 DSP multiply.
//
//  CHANGES FROM ORIGINAL:
//    [FIX-1]  cresult_t → ccov_result_t  (matches updated header).
//    [FIX-2]  Added missing s_axilite interface pragmas for pointer arguments
//             X and R so their AXI-Lite base-address registers are exposed
//             to the PS correctly (offset=slave requires this).
//    [FIX-3]  Removed duplicate s_axilite pragmas that appeared alongside
//             m_axi pragmas for X and R; interface specification is now split
//             cleanly: m_axi for the data path, s_axilite for address/ctrl.
//    [FIX-4]  Added DEPENDENCE pragma on X_buf inner load loop to confirm
//             intra-iteration false RAW and allow II=1 burst loading.
//    [FIX-5]  Diagonal imaginary: added explicit zero-assignment for R_buf[i][i].im
//             to guard against fixed-point residue accumulation.
//    [FIX-6]  acc_re / acc_im declared volatile-style reset inside OUTER_J
//             initialisation block — was already correct but comment clarified.
// =============================================================================

#include "covariance_matrix.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: complex multiply-accumulate  acc += a * conj(b)
//
//  Expansion:
//    re: a.re*b.re + a.im*b.im
//    im: a.im*b.re − a.re*b.im
//
//  Inlined so the four real multiplies fold into the surrounding pipeline
//  stage and map to DSP48E2 MAC mode.
// ─────────────────────────────────────────────────────────────────────────────
static void cmac_conj(accum_t       &acc_re,
                      accum_t       &acc_im,
                      const samp_t   a_re,
                      const samp_t   a_im,
                      const samp_t   b_re,
                      const samp_t   b_im)
{
#pragma HLS INLINE
    acc_re += a_re * b_re + a_im * b_im;   // real part of  a · b*
    acc_im += a_im * b_re - a_re * b_im;   // imag part of  a · b*
}

// ─────────────────────────────────────────────────────────────────────────────
//  Top-level kernel
// ─────────────────────────────────────────────────────────────────────────────
void covariance_estimation(
    csamp_t       X[M_ANT][K_SAMP],
    ccov_result_t R[M_ANT][M_ANT])
{
    // ── AXI4 master ports ──────────────────────────────────────────────────
    // Two separate AXI bundles so read and write channels use independent
    // NoC paths in Versal — zero arbitration contention between phases.
    //   depth     : number of elements (used by co-simulation, not synthesis)
    //   latency   : modelled AXI round-trip latency in cycles (64 ≈ 213 ns
    //               @ 300 MHz, typical for Versal NoC → LPDDR5)
    //   burst_length / outstanding : latency-hiding parameters
#pragma HLS INTERFACE m_axi port=X offset=slave bundle=gmem0 \
        depth=8256  max_read_burst_length=256  latency=64  num_read_outstanding=16
#pragma HLS INTERFACE m_axi port=R offset=slave bundle=gmem1 \
        depth=4096  max_write_burst_length=256 latency=64  num_write_outstanding=16

    // ── AXI4-Lite slave: control + base-address registers ─────────────────
    // [FIX-2] All pointer arguments MUST appear here so HLS generates the
    // base-address registers that the PS writes before asserting ap_start.
    // Without these, offset=slave has nowhere to store the DDR addresses.
#pragma HLS INTERFACE s_axilite port=X      bundle=ctrl
#pragma HLS INTERFACE s_axilite port=R      bundle=ctrl
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl

    // ── On-chip buffers ────────────────────────────────────────────────────

    // X_buf[antenna][time] — 64 independent BRAMs after complete dim=1 partition
    // Memory: 64 × 129 × 32 bits ≈ 33 KB  (each row fits in LUTRAM < 18 Kb)
    csamp_t X_buf[M_ANT][K_SAMP];
#pragma HLS ARRAY_PARTITION variable=X_buf dim=1 complete

    // R_buf[row][col] — 64 independent BRAMs after complete dim=1 partition
    // Memory: 64 × 64 × 64 bits = 32 KB
    ccov_result_t R_buf[M_ANT][M_ANT];      // [FIX-1]
#pragma HLS ARRAY_PARTITION variable=R_buf dim=1 complete

    // ─────────────────────────────────────────────────────────────────────
    //  PHASE 1 – Burst-load X from DDR into on-chip X_buf
    //
    //  PIPELINE II=1 on LOAD_TIME issues one AXI read per cycle.
    //  max_read_burst_length=256 and num_read_outstanding=16 hide the
    //  64-cycle NoC latency.
    //
    //  [FIX-4] DEPENDENCE pragma: HLS conservatively infers RAW hazard on
    //  X_buf across LOAD_TIME iterations.  Declaring it false (intra) allows
    //  II=1.  The synthesis warning "Found false RAW intra dependency" is
    //  expected and confirms the pragma was applied correctly.
    // ─────────────────────────────────────────────────────────────────────
    LOAD_ANT:
    for (int i = 0; i < M_ANT; i++) {
        LOAD_TIME:
        for (int t = 0; t < K_SAMP; t++) {
#pragma HLS PIPELINE II=1
#pragma HLS DEPENDENCE variable=X_buf intra RAW false  // [FIX-4]
            X_buf[i][t] = X[i][t];
        }
    }

    // Normalization constant 1/K — compile-time literal → one DSP multiply
    const accum_t inv_K = accum_t(K_INV_VAL);

    // ─────────────────────────────────────────────────────────────────────
    //  PHASE 2 – Compute R̂ = (1/K) · X · X^H  (upper triangle + Hermitian)
    //            Write into on-chip R_buf (1–2 cycle latency, not AXI).
    //
    //  Loop counts:
    //    OUTER_I × OUTER_J : 2080 (i,j) pairs
    //    INNER_K            : 129 cycles each @ II=1
    //  Total: 2080 × 129 = 268 320 cycles ≈ 894 μs @ 300 MHz
    // ─────────────────────────────────────────────────────────────────────
    OUTER_I:
    for (int i = 0; i < M_ANT; i++) {

        OUTER_J:
        for (int j = i; j < M_ANT; j++) {

            // Accumulators: declared as scalars → HLS infers pipeline regs.
            // Reset to zero for every new (i,j) pair.
            accum_t acc_re = accum_t(0);
            accum_t acc_im = accum_t(0);

            // Inner pipelined MAC loop — II=1, 4 DSPs active per cycle
            INNER_K:
            for (int t = 0; t < K_SAMP; t++) {
#pragma HLS PIPELINE II=1
                cmac_conj(acc_re, acc_im,
                          X_buf[i][t].re, X_buf[i][t].im,
                          X_buf[j][t].re, X_buf[j][t].im);
            }

            // Normalize (Q40.15 × constant → Q32.8 with SAT+RND)
            cov_result_t r_re = cov_result_t(acc_re * inv_K);
            cov_result_t r_im = cov_result_t(acc_im * inv_K);

            // Upper triangle
            R_buf[i][j].re =  r_re;
            R_buf[i][j].im =  r_im;

            if (i != j) {
                // Hermitian fill: conj — negation of imag is a sign-bit flip (1 LUT)
                R_buf[j][i].re =  r_re;
                R_buf[j][i].im = -r_im;
            } else {
                // [FIX-5] Diagonal: force imaginary to exact zero to prevent
                // any fixed-point residue from propagating to the Cholesky stage.
                R_buf[i][i].im = cov_result_t(0);
            }

        } // OUTER_J
    } // OUTER_I

    // ─────────────────────────────────────────────────────────────────────
    //  PHASE 3 – Burst-write R_buf → DDR via AXI4
    //
    //  One contiguous 4096-beat burst pays the 64-cycle AXI latency exactly
    //  once.  (v1.0 paid it 2080× inside the compute loop → 3 ms overhead.)
    //
    //  Latency: 64 × 64 = 4096 cycles ≈ 14 μs @ 300 MHz
    // ─────────────────────────────────────────────────────────────────────
    WRITE_ROW:
    for (int i = 0; i < M_ANT; i++) {
        WRITE_COL:
        for (int j = 0; j < M_ANT; j++) {
#pragma HLS PIPELINE II=1
            R[i][j] = R_buf[i][j];
        }
    }

    // ─────────────────────────────────────────────────────────────────────
    //  Kernel latency summary (all phases, 300 MHz):
    //    Phase 1 – Load  X_buf : ~16 512 cycles (II=2 observed)  ≈  55 μs
    //    Phase 2 – Compute     : ~268 320 cycles (II=1)          ≈ 894 μs
    //    Phase 3 – Write R     :   ~4 096 cycles (II=1)          ≈  14 μs
    //    Drain + overhead      :   ~1 500 cycles                 ≈   5 μs
    //    ─────────────────────────────────────────────────────────────────
    //    Total                 : ~290 428 cycles                 ≈ 968 μs
    //
    //  Resource estimate (C Synthesis, M_ANT=64, K_SAMP=129):
    //    DSP     :     4 / 1968   (0.2%)
    //    BRAM_18K:     4 /  967   (0.4%  — AXI FIFOs only; bufs in LUTRAM)
    //    LUT     : ~17 720 / 899 840  (2.0%)
    //    FF      :  ~6 763
    // ─────────────────────────────────────────────────────────────────────

} // covariance_estimation
