// rdbs_aie.cc
// =============================================================================
// RDBS Beamspace Compressor — AI Engine Kernel Implementation
// =============================================================================
// Computes 21 complex dot products (64 taps each) per snapshot using the
// AIE vector unit. This replaces rdbs_kernel.cpp's BEAM_LOOP + DOT_PRODUCT
// (which used #pragma HLS UNROLL on 64 PL DSPs) with AIE's native 256-bit
// vector MAC, which processes 8 cint16 lanes per cycle.
//
// AIE API used: aie::accum, aie::mul, aie::load, vector intrinsics from
// aie_api (the portable AIE API — works across AIE1/AIE2 architectures,
// which VCK190's AIE-ML/AIE1 tiles support).
// =============================================================================

#include "rdbs_aie.h"

void rdbs_beamform(
    input_window_cint16  * __restrict x_in,
    output_window_cint16 * __restrict y_out,
    const cint16 (*coeff_re_im)[M_ELEMENTS])
{
    // ── Load the full 64-sample snapshot into a local buffer ────────────
    // window_readincr loads cint16 values from the window stream into
    // local AIE vector registers / scratch memory. 64 samples = 8 vectors
    // of 8 cint16 each (AIE-ML vector lane width = 8 x cint16 per 256-bit reg).
    alignas(32) cint16 x_local[M_ELEMENTS];

    for (unsigned m = 0; m < M_ELEMENTS; m++) {
#pragma unroll
        x_local[m] = window_readincr(x_in);
    }

    // ── Compute 21 beams: y[b] = sum_m W_H[b][m] * x[m] ──────────────────
    // Each beam is an independent 64-tap complex dot product.
    // aie::mul on cint16 vectors performs complex multiply (re*re - im*im,
    // re*im + im*re) and accumulates into a 48-bit-wide accumulator native
    // to the AIE MAC array, so no manual real/imag bookkeeping is needed
    // the way cmac_conj() did in the PL version.
    BEAM_LOOP:
    for (unsigned b = 0; b < B_BEAMS; b++) {

        // aie::vector<cint16,8> holds 8 complex taps per register.
        // 64 taps / 8 lanes = 8 vector MAC operations per beam.
        ::aie::accum<cacc48, 8> acc_vec;
        acc_vec.from_int(0);

        DOT_PRODUCT_VEC:
        for (unsigned v = 0; v < M_ELEMENTS / 8; v++) {
            // Load 8 taps of input and 8 taps of this beam's coefficient row
            ::aie::vector<cint16, 8> xv  = ::aie::load_v<8>(&x_local[v * 8]);
            ::aie::vector<cint16, 8> wv  = ::aie::load_v<8>(&coeff_re_im[b][v * 8]);

            // Complex multiply-accumulate, 8 lanes in parallel
            acc_vec = ::aie::mac(acc_vec, xv, wv);
        }

        // Horizontal-sum the 8 partial accumulator lanes down to 1 scalar.
        // This is the cross-lane reduction step — AIE MACs 8 products in
        // parallel per cycle but the 8 results must still be summed together
        // since all 8 belong to the SAME beam's dot product.
        cacc48 acc_scalar = ::aie::reduce_add(acc_vec);

        // Shift right by RDBS_SHIFT (18 bits) to rescale from the Q30
        // intermediate product down to Q12 (the ap_fixed<16,4> output
        // format), then saturate to int16 range. This replaces the
        // implicit ap_fixed truncation/rounding the PL version got for
        // free from (out_data_t)acc_real / (out_data_t)acc_imag.
        cint16 y_b = acc_scalar.to_int(RDBS_SHIFT);   // rounds + saturates

        window_writeincr(y_out, y_b);
    }

    // ── Pad output window to 24 elements for 32-byte alignment ──────────
    // The Covariance AIE kernel (Stage B) only reads the first 21 values;
    // these 3 padding writes exist purely so the AIE compiler can emit an
    // aligned vector store on the output window instead of a slower
    // unaligned scalar store for the tail.
    PAD_LOOP:
    for (unsigned p = B_BEAMS; p < RDBS_OUT_PADDED; p++) {
        window_writeincr(y_out, cint16(0, 0));
    }
}
