// rdbs_aie.h
// =============================================================================
// RDBS Beamspace Compressor — AI Engine Kernel
// =============================================================================
// AIE-native re-implementation of rdbs_kernel.cpp / rdbs_kernel.h.
// Computes the same operation: y[b] = sum_m W_H[b][m] * x[m], for b = 0..20
// 64-element input -> 21-beam output, per snapshot, 1000 snapshots total.
//
// DATA TYPE NOTE — read this before touching the rest of the file:
// ─────────────────────────────────────────────────────────────────
// The original PL kernel uses ap_fixed<16,1> for x and W_H, ap_fixed<16,4>
// for output, ap_fixed<40,8> for the accumulator. AIE's native vector
// datapath does NOT have an ap_fixed equivalent — the AIE compute units
// work on int16/int32/cint16/cint32 (cint = complex int) with an explicit
// SHIFT applied after multiply-accumulate to emulate fixed-point scaling.
//
// Mapping used here:
//   ap_fixed<16,1>  (1 int bit, 15 frac bits)  -> cint16, shift = 15
//   ap_fixed<16,4>  (4 int bit, 12 frac bits)  -> cint16, shift = 12
//   ap_fixed<40,8>  accumulator                -> AIE's native 48-bit
//                                                  accumulator (acc48),
//                                                  no struct needed — the
//                                                  AIE MAC intrinsics
//                                                  accumulate internally
//                                                  in 48-bit precision
//                                                  regardless of the I/O
//                                                  type width.
//
// This means: the BIT PATTERN on the wire (ap_fixed<16,1> as int16) is
// IDENTICAL to what AIE treats as a plain int16. No reinterpretation is
// needed at the PL<->AIE boundary — only the SHIFT amount (which exponent
// the fixed point sits at) must match, and that is a compile-time constant
// passed to the kernel, not a runtime bit operation.
// =============================================================================
#ifndef RDBS_AIE_H
#define RDBS_AIE_H

#include <adf.h>
#include <aie_api/aie_adf.hpp>

// ─── Dimensions (must match rdbs_params.h exactly) ───────────────────────────
#define M_ELEMENTS   64
#define B_BEAMS      21
#define K_SNAPSHOTS  1000

// ─── Fixed-point shift amounts (replace ap_fixed frac-bit counts) ───────────
// Input x:      ap_fixed<16,1>  -> 15 fractional bits
// Coefficients: ap_fixed<16,1>  -> 15 fractional bits
// Output y:     ap_fixed<16,4>  -> 12 fractional bits
//
// Product of two Q15 numbers lands at Q30 inside the 48-bit accumulator.
// To produce a Q12 result, shift right by (15 + 15 - 12) = 18.
#define RDBS_SHIFT   18

// ─── Per-call window sizes ────────────────────────────────────────────────
// One AIE kernel invocation processes ONE snapshot:
//   input window:  64 complex int16 samples  (cint16, 4 bytes each = 256B)
//   output window: 21 complex int16 beams    (cint16, 4 bytes each = 84B)
//
// AIE windows must be 32-byte aligned for vector load/store efficiency.
// 64 cint16 = 256 bytes -> aligned. 21 cint16 = 84 bytes -> NOT aligned to
// 32B; the graph.cpp pads the output window to 24 elements (96 bytes) to
// keep the vector store aligned. The extra 3 elements are written but
// unused by the downstream consumer (Covariance AIE kernel only reads
// the first 21).
#define RDBS_IN_SAMPLES   M_ELEMENTS      // 64
#define RDBS_OUT_BEAMS    B_BEAMS         // 21
#define RDBS_OUT_PADDED   24              // padded to keep 32B alignment

// ─── Kernel function ─────────────────────────────────────────────────────
// Called once per snapshot by the AIE graph's run() loop (K_SNAPSHOTS times).
// coeff_re / coeff_im are loaded ONCE at graph init via a separate
// init-time RTP (run-time parameter) write, not re-sent every call —
// see graph.cpp for how this is wired.
void rdbs_beamform(
    input_window_cint16  * __restrict x_in,    // 64 complex int16 samples
    output_window_cint16 * __restrict y_out,   // 24 complex int16 (21 valid + 3 pad)
    const cint16 (*coeff_re_im)[M_ELEMENTS]    // [B_BEAMS][M_ELEMENTS], packed W_H
);

#endif // RDBS_AIE_H
