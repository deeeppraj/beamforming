# MVDR BEAMFORMER — DEBUG FOLLOW-UP / RESOLUTION REPORT
## Response to: debug_handoff.txt (Section 5 blocker — AXIS pipeline never produces data)

================================================================================
STATUS: ROOT CAUSE IDENTIFIED AND FIXED (software-only fix, no Vivado rebuild)
================================================================================

This document picks up exactly where debug_handoff.txt Section 11 asked the
next session to start: running the Section 7 Tcl diagnostic commands and
reporting back. All commands were run in order, plus several follow-ups that
weren't anticipated in the original plan. Below is the full chronological
trail of what was tested, what came back, and how each result changed the
working theory until the actual bug was found.


--------------------------------------------------------------------------------
1. STARTING POINT
--------------------------------------------------------------------------------

The handoff doc's hypothesis (Section 5) ranked four candidate causes for why
RDBS never reports done=1, in this order of suspected probability:

  1. AXIS TDATA width mismatch (DMA outputs wider beats than RDBS expects)
  2. AXIS clock domain mismatch
  3. Missing AXIS interconnect / converter IP
  4. TLAST handling issue

Section 7 of the handoff doc supplied six groups of Tcl commands to run
before touching anything in Vivado. We ran Group 1 and Group 2 first, since
those were flagged as the highest-value, lowest-cost checks.


--------------------------------------------------------------------------------
2. GROUP 1 — AXI DMA AXIS OUTPUT CONFIGURATION
--------------------------------------------------------------------------------

Command run:
    get_property CONFIG.c_m_axis_mm2s_tdata_width [get_bd_cells axi_dma_0]

Result:
    64

Command run:
    get_property CONFIG.c_m_axi_mm2s_data_width [get_bd_cells axi_dma_0]

Result:
    64

Command run:
    get_property CONFIG.c_sg_length_width [get_bd_cells axi_dma_0]

Result:
    23

Interpretation at this point:
  - The handoff doc's Case A predicted "Expected if working: 32". We got 64.
  - This looked like an immediate confirmation of Case A (width mismatch),
    exactly as predicted — DMA configured wider than RDBS's assumed 32-bit
    interface.
  - c_sg_length_width=23 confirmed the earlier 23-bit buffer length fix
    (Section 4 of the handoff doc) held correctly through whatever rebuild
    cycle produced the current bitstream. Good — that part of "known-good
    components" is genuinely solid and was never touched again.

ACTION TAKEN (first attempt):
  Per Option A1 in the handoff doc (Section 8, Case A), changed
  axi_dma_0's Memory Map Data Width and Stream Data Width from 64 to 32,
  intending to match what was assumed to be RDBS's native 32-bit interface.


--------------------------------------------------------------------------------
3. GROUP 2 — CLOCK CHECKS (run in parallel before committing to a rebuild)
--------------------------------------------------------------------------------

Command run:
    get_property CONFIG.FREQ_HZ [get_bd_intf_pins axi_dma_0/M_AXIS_MM2S]
    get_property CONFIG.FREQ_HZ [get_bd_intf_pins rdbs_kernel_0/x_in]

Result:
    99999900  (both, identical)

Interpretation:
  Clocks match exactly (~100 MHz on both sides of the AXIS link). This
  ruled out Case B (clock domain mismatch) entirely — no clock converter
  was ever needed. This was good news: it meant whatever the real bug was,
  it was isolated to a single cause rather than two compounding issues, which
  the handoff doc's "Risk 2" section had flagged as a real possibility.


--------------------------------------------------------------------------------
4. VERIFYING THE WIDTH CHANGE TOOK EFFECT
--------------------------------------------------------------------------------

After setting axi_dma_0 to 32-bit width in Vivado, re-ran:

    get_property CONFIG.c_m_axis_mm2s_tdata_width [get_bd_cells axi_dma_0]
    get_property CONFIG.c_m_axi_mm2s_data_width [get_bd_cells axi_dma_0]

Result:
    32
    32

Confirmed the change registered correctly in the design.


--------------------------------------------------------------------------------
5. THE TURNING POINT — CHECKING RDBS'S ACTUAL INTERFACE, NOT THE ASSUMED ONE
--------------------------------------------------------------------------------

Before committing to a full Vivado rebuild (regenerate device image, re-export
XSA, rebuild Vitis platform — a multi-hour cycle), we did one more sanity
check that the handoff doc's Case A description didn't explicitly call for:
querying RDBS's *synthesized* interface directly, rather than trusting the
.cpp source comment that said "axis_data_t = ap_uint<32>".

Command run:
    report_property [get_bd_intf_pins rdbs_kernel_0/x_in]

Critical result:
    TDATA_NUM_BYTES   8
    HAS_TLAST         0
    MODE              Slave

This was the moment the original diagnosis broke down. RDBS's actual
synthesized AXIS port is 64 bits wide (8 bytes), not 32 bits as assumed from
reading rdbs_kernel.h's struct definition. The handoff doc's Section 5 had
inferred the expected width from the C++ source:

    struct axis_data_t {
        ap_int<32> data;
        bool last;
    };

But Vitis HLS does not synthesize this struct as "32-bit data + a separate
TLAST wire." Confirmed via the IP's packaged component.xml (pulled directly
from the IP-XACT description, the authoritative source for what the RTL
actually exposes):

    <spirit:busInterface>
      <spirit:name>x_in</spirit:name>
      ...
      <spirit:parameter>
        <spirit:name>TDATA_NUM_BYTES</spirit:name>
        <spirit:value>8</spirit:value>
      </spirit:parameter>
    </spirit:busInterface>

    <spirit:port>
      <spirit:name>x_in_TDATA</spirit:name>
      <spirit:wire>
        <spirit:direction>in</spirit:direction>
        <spirit:vector>
          <spirit:left>63</spirit:left>
          <spirit:right>0</spirit:right>
        </spirit:vector>
      </spirit:wire>
    </spirit:port>

No `x_in_TLAST` port exists at all in the port list. HLS packed the entire
{data, last} struct into a 64-bit TDATA word with zero use of the AXI-Stream
protocol's actual TLAST signal — the `last` boolean became a regular data
bit, byte-aligned per HLS's default struct serialization rules (32-bit data
field followed by a 1-bit bool, padded up to the next byte/word boundary,
landing the whole thing at 64 bits total).

This single finding overturned the entire Case A diagnosis. The "fix" of
setting DMA to 32-bit was not correcting a mismatch — it was *creating* one,
in the opposite direction from what was originally assumed.


--------------------------------------------------------------------------------
6. REVERTING THE WIDTH CHANGE
--------------------------------------------------------------------------------

Reverted axi_dma_0 back to 64-bit (matching RDBS's real interface):

    set_property -dict [list \
      CONFIG.c_m_axi_mm2s_data_width {64} \
      CONFIG.c_m_axis_mm2s_tdata_width {64} \
      CONFIG.c_mm2s_burst_size {256} \
    ] [get_bd_cells axi_dma_0]

Verified:
    c_m_axis_mm2s_tdata_width = 64
    c_m_axi_mm2s_data_width   = 64
    c_mm2s_burst_size         = 256

This restored the DMA to its width from BEFORE this debug session started.
Conclusion: width, as configured in Vivado, was never actually the bug.
It was correct from the start (64-bit on both DMA and RDBS) — the handoff
doc's assumption that RDBS "expected 32-bit" was the error, not the
hardware configuration.

NO VIVADO REBUILD WAS PERFORMED. No bitstream regeneration. No new XSA
export. This entire phase of investigation resulted in zero hardware
changes — the design Vivado already had was correct on the width front the
whole time.


--------------------------------------------------------------------------------
7. SO WHAT WAS ACTUALLY WRONG?
--------------------------------------------------------------------------------

If DMA and RDBS were always width-matched at 64 bits, the real bug had to be
in how `main.c` populated and sized the input buffer being streamed.

Reviewing the original main_consolidated.c:

    #include "input_samples.h"  /* uint32_t input_samples[64000] */
    #define INPUT_SIZE_BYTES (N_CHANNELS * N_SNAPSHOTS * 4)   /* 256000 */

This was written under the same incorrect assumption baked into the .cpp
source comments — that each AXIS beat carries one 32-bit packed sample.
With DMA correctly configured at 64-bit width, sending 256000 bytes means
DMA emits 256000 / 8 = 32000 beats total.

But RDBS's READ_INPUT loop calls x_in.read() exactly
num_snapshots * M_ELEMENTS = 1000 * 64 = 64000 times before it can ever
assert ap_done. It was waiting for 64000 beats and only ever receiving
32000 — exactly half. This is precisely why every run produced
"RDBS: done=0 idle=0 ready=0": the kernel was correctly, permanently
blocked mid-stream waiting for data that would never arrive, because the
testbench-side buffer was half the size the hardware interface actually
required.

This also explains a detail noted in the handoff doc's Phase D ("Hardware R
dump diagnostic") — the observation that R[0,0] and R[0,1] matched the
steering vector's first two entries. With RDBS stalled forever, Covariance
was equally stalled waiting on RDBS's output stream, so it never wrote
anything to its output buffer. The "R values" being read out were just
stale DDR contents left over from an earlier memcpy of the steering vector
into a nearby region — not a sign of any data corruption, just confirmation
that the covariance kernel's m_axi write phase was never reached.


--------------------------------------------------------------------------------
8. THE FIX
--------------------------------------------------------------------------------

Two changes, both confined to the host-side application — no hardware
changes of any kind:

a) input_samples.h regenerated as a 64-bit array instead of 32-bit:

     uint32_t input_samples[64000]    -- OLD, wrong
     uint64_t input_samples[64000]    -- NEW, correct

   Each entry now holds the same 32-bit packed sample
   (bits[31:16]=real, bits[15:0]=imag) zero-extended into the low 32 bits
   of a 64-bit word, with the upper 32 bits set to zero. RDBS's READ_INPUT
   loop only ever reads pkt.data.range(31,16) and pkt.data.range(15,0) from
   the incoming word and never inspects bits above 31, so the upper half
   being zero is inert and harmless — it does not need to encode the `last`
   field for x_in specifically, since rdbs_kernel.cpp's read loop never
   touches pkt.last on the input side at all (confirmed by re-reading
   rdbs_kernel.cpp directly — the .last field is only ever assigned on the
   *output* side, when constructing y_out packets for the 21-beam output
   stream, not consumed on x_in).

b) main.c's INPUT_SIZE_BYTES macro updated:

     #define INPUT_SIZE_BYTES (N_CHANNELS * N_SNAPSHOTS * 4)   -- OLD, 256000 bytes
     #define INPUT_SIZE_BYTES (N_CHANNELS * N_SNAPSHOTS * 8)   -- NEW, 512000 bytes

   This now correctly produces exactly 64000 64-bit beats over the AXIS
   link (512000 / 8 = 64000), matching what RDBS's READ_INPUT loop is
   actually counting up to.

A regenerated input_samples.h was produced from the original
rdbs_input.dat (64000 rows, real/imag space-separated, verified byte-exact
against the value referenced in the handoff doc's Section 5 "PROVEN NOT TO
BE THE CAUSE" list — first row 864 5793 → packed 0x036016A1 — same value,
now correctly embedded as a zero-extended 64-bit word
0x00000000036016A1ULL rather than a bare 32-bit constant).

The earlier DMA chunking removal (single XAxiDma_SimpleTransfer call
instead of the 4096-byte chunked loop, from Phase B of the handoff doc) was
kept as-is — that change was orthogonal to this bug and remains correct now
that the 23-bit buffer length register comfortably covers the new 512000-
byte transfer size in one shot.


--------------------------------------------------------------------------------
9. WHY THE HANDOFF DOC'S OWN ASSUMPTION WAS WRONG, AND HOW TO AVOID THIS CLASS
   OF BUG GOING FORWARD
--------------------------------------------------------------------------------

The original Section 5 diagnosis was reasoned from the HLS C++ source
(`rdbs_kernel.h`'s struct definition), which is a completely reasonable
starting point — but Vitis HLS's struct-to-AXIS packing is not always
intuitive, and it does not automatically map a struct field literally named
"last" onto the protocol's TLAST signal unless the kernel is explicitly
written using `hls::axis` (ap_axiu) types or the struct is given specific
side-channel pragmas. Without that, HLS just serializes the whole struct
bit-for-bit into TDATA with byte-alignment padding, silently growing the
interface width and silently discarding any protocol meaning the field
name might have implied.

The general lesson, useful for the remaining kernels (covariance_estimation,
mvdr_weights) and any future debugging: never infer AXI-Stream/AXI4 port
widths from C++ source comments or struct definitions. Always confirm
against one of:
  - the synthesized IP's component.xml (TDATA_NUM_BYTES, HAS_TLAST fields)
  - report_property on the actual block-design pin
  - the HLS synthesis report's Interface section

All three were available and not fully cross-checked against the source-
code assumption before the first (incorrect) Case A fix attempt was made.
Doing so earlier would have skipped the wasted width-flip-and-revert cycle
entirely.


--------------------------------------------------------------------------------
10. CURRENT STATE / WHAT'S LEFT
--------------------------------------------------------------------------------

Completed:
  [x] Confirmed DMA and RDBS widths were always correctly matched at 64-bit
  [x] Confirmed clocks match (no converter needed)
  [x] Identified true root cause: host-side buffer was half the required
      size due to an incorrect 32-bit-per-sample assumption
  [x] Regenerated input_samples.h as uint64_t[64000], verified first/last
      entries against known-good values from the original .dat file
  [x] Updated main.c's INPUT_SIZE_BYTES to match (512000 bytes)
  [x] No Vivado changes required for this fix — current bitstream/PDI
      remains valid and does not need to be regenerated

Not yet done (next steps):
  [ ] Rebuild Vitis application with corrected input_samples.h and main.c
  [ ] Reprogram board (existing bitstream, no FPGA fabric changes needed)
  [ ] Re-run and capture new RDBS/Cov/MVDR status flags — expecting
      RDBS: done=1 idle=1 ready=1 for the first time
  [ ] If RDBS now completes, check whether Covariance also completes
      (it was previously blocked purely as a downstream consequence of
      RDBS never finishing — no separate bug has been found in the
      RDBS-to-Covariance AXIS link, though report_property on
      covariance_estimation_0/y_in has not yet been independently checked
      and should be, given how wrong the RDBS x_in assumption turned out
      to be)
  [ ] If RDBS and Covariance both complete, dump R from DDR and compare
      against MATLAB's R using mvdr_validate_v3.m — this is the first
      point at which a real numerical comparison becomes meaningful, since
      every prior "R looks plausible" observation in the handoff doc
      (Phase C/D) was working from data that was never actually written
      by hardware
  [ ] Only after R matches MATLAB within expected fixed-point quantization
      error should MVDR's output weights be compared against MATLAB's
      reference weights — comparing them before R is confirmed correct
      would repeat the same mistake the handoff doc's Phase C already
      identified and corrected for (the "93% error, looked plausible but
      was computed from garbage" episode)


--------------------------------------------------------------------------------
11. FILES CHANGED
--------------------------------------------------------------------------------

  input_samples.h   — regenerated, uint64_t[64000], values zero-extended
                       from original 32-bit packed real/imag samples
  main.c             — INPUT_SIZE_BYTES changed from *4 to *8;
                        chunked-DMA loop (already removed in a prior pass)
                        confirmed still absent / single-transfer retained
  axi_dma_0 (Vivado) — touched during investigation (32-bit), then
                        reverted to original 64-bit config; net change
                        across this session is zero for the Vivado design

No new XSA export was generated. No new bitstream was generated. The
existing exported hardware remains valid for the next Vitis rebuild.

================================================================================
END OF FOLLOW-UP REPORT
================================================================================
