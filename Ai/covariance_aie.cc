// covariance_aie.cc
// =============================================================================
// Covariance AIE Kernel Implementation — Single tile's worth of pair compute
// =============================================================================
// ⚠ UNVALIDATED — see covariance_aie.h header for full risk discussion.
// The biggest unverified assumption in this file: that the FULL X_buf
// (21 channels x 1000 snapshots x cint16 = 84,000 bytes) fits in one AIE
// tile's local data memory alongside this kernel's working variables and
// the AIE compiler's own stack/buffer overhead. AIE-ML tiles on VCK190
// typically have 64KB of local data memory PER TILE. 84,000 bytes alone
// is already inside that 64KB budget (~82KB used, leaving only ~46KB free
// once you count double-buffering for ping-pong windows), so this is
// TIGHT, not comfortable. If the AIE compiler reports memory overflow,
// the fix is to split X_buf delivery into per-channel windows streamed
// incrementally rather than one giant window — that restructuring is
// NOT implemented here and is flagged as required follow-up work.
// =============================================================================

#include "covariance_aie.h"

void covariance_tile_kernel(
    input_window_cint16  * __restrict X_buf_window,
    const pair_idx_t     * __restrict pair_list,
    unsigned                          n_pairs,
    output_window_cint32 * __restrict r_out)
{
    // ── Load the full X buffer into local scratch ───────────────────────
    // X_local[ch][t]: same indexing as covariance_matrix.cpp's X_buf[ch][t].
    // NOTE: 21 * 1000 = 21000 cint16 elements = 84,000 bytes. See header
    // warning above — this is the part that needs simulator confirmation.
    alignas(32) cint16 X_local[M_CH][K_SAMP];

    LOAD_LOOP:
    for (unsigned ch = 0; ch < M_CH; ch++) {
        for (unsigned t = 0; t < K_SAMP; t++) {
#pragma unroll(8)
            X_local[ch][t] = window_readincr(X_buf_window);
        }
    }

    // ── Compute this tile's assigned (i,j) pairs ─────────────────────────
    // Each pair: R[i][j] = (1/K) * sum_t X[i][t] * conj(X[j][t])
    //
    // conj(X[j][t]) is handled the SAME way cmac_conj() did in the PL
    // version: for complex a = (a_re, a_im), b = (b_re, b_im),
    //   a * conj(b) = (a_re*b_re + a_im*b_im) + j*(a_im*b_re - a_re*b_im)
    // AIE's aie::mul on cint16 computes STANDARD complex multiply
    // (a*b, not a*conj(b)), so this kernel manually negates b_im before
    // the multiply to get the conjugate effect -- there is no native
    // "multiply-by-conjugate" intrinsic in the portable aie_api used here.
    PAIR_LOOP:
    for (unsigned p = 0; p < n_pairs; p++) {
        unsigned ch_i = pair_list[p].i;
        unsigned ch_j = pair_list[p].j;

        // Running scalar accumulator (48-bit native AIE accumulator).
        // NOTE: this version processes 1 sample/cycle (no vectorization
        // across the K_SAMP dimension) because each pair needs its OWN
        // accumulator and conj(X[j]) must be formed per-sample. A faster
        // version would vectorize 8 snapshots at a time using
        // aie::vector<cint16,8> loads from X_local[ch_i] and X_local[ch_j]
        // plus a per-lane conjugate-negate step -- THIS OPTIMIZATION IS
        // NOT YET IMPLEMENTED. Treat this as a functionally-correct but
        // not yet performance-tuned first pass.
        int64_t acc_re = 0;
        int64_t acc_im = 0;

        MAC_LOOP:
        for (unsigned t = 0; t < K_SAMP; t++) {
            int32_t a_re = X_local[ch_i][t].real;
            int32_t a_im = X_local[ch_i][t].imag;
            int32_t b_re = X_local[ch_j][t].real;
            int32_t b_im = X_local[ch_j][t].imag;

            // a * conj(b): re = a_re*b_re + a_im*b_im,  im = a_im*b_re - a_re*b_im
            acc_re += (int64_t)a_re * b_re + (int64_t)a_im * b_im;
            acc_im += (int64_t)a_im * b_re - (int64_t)a_re * b_im;
        }

        // ── Normalize by 1/K ──────────────────────────────────────────
        // K_SAMP = 1000 is not a power of 2 (see header note), so this
        // is a true multiply by a precomputed Q15 reciprocal constant,
        // matching the PL version's accum_t(1.0 / K_SAMP) behavior,
        // NOT a bit-shift.
        int64_t r_re_scaled = (acc_re * INV_K_Q15) >> (15 + COV_SHIFT);
        int64_t r_im_scaled = (acc_im * INV_K_Q15) >> (15 + COV_SHIFT);

        // Saturate to int32 range before packing into cint32 output
        if (r_re_scaled >  2147483647LL) r_re_scaled =  2147483647LL;
        if (r_re_scaled < -2147483648LL) r_re_scaled = -2147483648LL;
        if (r_im_scaled >  2147483647LL) r_im_scaled =  2147483647LL;
        if (r_im_scaled < -2147483648LL) r_im_scaled = -2147483648LL;

        cint32 r_val;
        r_val.real = (int32_t)r_re_scaled;
        r_val.imag = (int32_t)r_im_scaled;

        window_writeincr(r_out, r_val);
    }
}
