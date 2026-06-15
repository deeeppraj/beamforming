# MVDR Beamformer: Vitis HLS → IP Catalog → Vivado Integration Guide
**Target:** Xilinx Versal VCK190 (`xcvc1902-vsva2197-2MP-e-S`) | **Tool:** Vitis HLS / Vivado 2024.1 | **Clock:** 300 MHz

This guide covers exporting the two HLS kernels — `covariance_estimation` and
`givens_qr_top` — as Vivado IP, and wiring them together (and to the Versal
CIPS/NoC) inside a Vivado IP Integrator block design.

---

## 0. Compatibility issues to resolve *before* you export anything

Based on the source review, the two kernels are **not yet wire-compatible**.
Fix these first — exporting before fixing them just produces IP you'll have
to re-export anyway.

| # | Issue | Where | Fix |
|---|-------|-------|-----|
| 1 | `givens_qr.h` still has `#define M 8` (dev value); `covariance_matrix.h` uses `M_ANT = 64`. | `givens_qr.h` | Change `#define M 8` → `#define M 64`. |
| 2 | `t_accum` is `ap_fixed<34,9,...>`, sized for `M=8`. FIX-3's own comment says it needs 12 integer bits for `M=64`. | `givens_qr.h` | Change to `ap_fixed<37,12,AP_RND,AP_SAT>` (or similar — keep ≥12 int bits, preserve fractional precision). |
| 3 | `cplx_rxx` (`hls::x_complex<t_rxx>`, `ap_fixed<32,8>`) is defined to match `cov_result_t` but is **not used by any function signature** in `givens_qr.h`. There is no Cholesky kernel that turns `R` (covariance output) into `L`/`Lt` (Givens-QR input). | Both | Decide which **Architecture** (A or B, §9) you'll use, and implement/locate the Cholesky stage accordingly. This is the actual integration gap — everything else is secondary to it. |
| 4 | `cov_result_t = ap_fixed<32,8,AP_RND_CONV,AP_SAT>` vs `t_rxx = ap_fixed<32,8,AP_RND,AP_SAT>` — same width, different rounding mode. | Both | If/when `cplx_rxx` is actually used (e.g., in a Cholesky kernel's input port), change `t_rxx` rounding to `AP_RND_CONV` to match `cov_result_t` bit-for-bit, or explicitly document the intended re-rounding. |
| 5 | `givens_qr.h` FIX-4 *reminds* that `givens_qr_top`'s array ports (`Lt`, `L`, `a_s`, `w_out`) need interface pragmas in `givens_qr.cpp` (`s_axilite` for control, plus `m_axi` or `bram`/`ap_memory` for data). **I could not verify `givens_qr.cpp` contains these** — it was not available for review. | `givens_qr.cpp` | Open `givens_qr.cpp` and confirm/add: <br>• `#pragma HLS INTERFACE s_axilite port=return bundle=ctrl` (and for each array, if you want PS-writable base addresses)<br>• `#pragma HLS INTERFACE m_axi port=Lt|L|a_s|w_out ...` if these should be DDR-mapped (Architecture A), **or** leave as `bram`/`ap_memory` if they'll be directly wired to BRAMs/FIFOs from `covariance_estimation`/a Cholesky IP (Architecture B). |

Everything from §1 onward assumes issues 1–5 have been addressed and `M=64`
on both sides.

---

## 1. System architecture overview

```mermaid
flowchart LR
    DDR[(DDR / LPDDR5<br/>via NoC)]
    CIPS[Versal CIPS<br/>PS]
    COV[covariance_estimation<br/>HLS IP<br/>X -> R]
    CHOL[Cholesky kernel<br/>R -> L, Lt<br/>(3rd party / PS / new HLS IP)]
    QR[givens_qr_top<br/>HLS IP<br/>L, Lt, a_s -> w_out]

    CIPS -- AXI4-Lite (ctrl) --> COV
    CIPS -- AXI4-Lite (ctrl) --> QR
    COV -- m_axi gmem0/gmem1 --> DDR
    DDR -- X --> COV
    COV -- R --> DDR
    DDR -- R --> CHOL
    CHOL -- L, Lt --> DDR
    DDR -- L, Lt, a_s --> QR
    QR -- w_out --> DDR
    CIPS -- reads w_out --> DDR
```

Two HLS IPs are built independently and dropped into one Vivado block
design. Because there is currently **no Cholesky-decomposition kernel** in
the project, §9 below gives two ways to close that gap.

---

## 2. Vitis HLS: build and validate each kernel

Do this for **both** `covariance_estimation` and `givens_qr_top` before
exporting anything — RTL co-simulation must pass first, otherwise the
exported IP will fail Vivado simulation/synthesis for the same reasons.

### 2.1 `covariance_estimation`

1. In Vitis HLS (Vitis Unified IDE → **Component → New Component → HLS**),
   create a component, e.g. `cov_est_hls`.
2. **Sources:** add `covariance_matrix.cpp`, `covariance_matrix.h`.
3. **Testbench:** add `tb_covariance_matrix.cpp`.
4. **Top function:** `covariance_estimation`.
5. **Part:** `xcvc1902-vsva2197-2MP-e-S`.
6. **Clock period:** `3.33` ns (300 MHz), uncertainty default (12.5%) is fine.
7. Run **C SIMULATION** — confirm `tb_covariance_matrix.cpp` prints
   `ALL TESTS PASSED`.
8. Run **C SYNTHESIS**. Check the report:
   - Confirm `m_axi` bundles `gmem0` (X, read) and `gmem1` (R, write) appear
     in the interface summary.
   - Confirm `s_axilite` bundle `ctrl` contains `X`, `R`, and `return`
     (these become the AXI4-Lite control register offsets you'll need for
     PS software).
   - Note the reported latency (~290k cycles per FIX comments) for
     scheduling/throughput planning.
9. Run **C/RTL CO-SIMULATION** (Verilog or VHDL, your preference — Verilog
   is the default and is what the rest of this guide assumes). It must pass.

### 2.2 `givens_qr_top`

1. Create a second component, e.g. `givens_qr_hls`.
2. **Sources:** add `givens_qr.cpp`, `givens_qr.h` (with the §0 fixes applied).
3. **Testbench:** add `tb_givens_qr.cpp`.
4. **Top function:** `givens_qr_top`.
5. **Part / clock:** same as above (`xcvc1902-vsva2197-2MP-e-S`, 3.33 ns).
6. Run **C SIMULATION** — confirm the testbench passes for `M=64`. (If the
   testbench was written for `M=8`, you may need to update its array sizes
   and any reference-model loop bounds to `M=64` as well — check
   `tb_givens_qr.cpp` for hardcoded `8`s.)
7. Run **C SYNTHESIS**. In the interface report, record exactly what each
   port (`Lt`, `L`, `a_s`, `w_out`) became:
   - If they show up as `m_axi` + `s_axilite` → Architecture A path (DDR-mapped).
   - If they show up as `ap_memory`/`bram` → Architecture B path (direct
     BRAM/FIFO connection to an upstream kernel).
   This determines how you wire it in §8.
8. Run **C/RTL CO-SIMULATION** and confirm pass.

---

## 3. Exporting RTL as Vivado IP (IP-XACT)

Repeat for each component (`cov_est_hls`, `givens_qr_hls`).

### 3.1 GUI method (Vitis Unified IDE)

1. With the component active and synthesis complete, go to the
   **Flow Navigator → IMPLEMENTATION → Package**, or use
   **Solution → Export RTL** (older HLS GUI naming) / **Run Package** in the
   newer Component flow.
2. Set packaging fields:
   - **Vendor:** e.g. `mycompany` (lowercase, no spaces)
   - **Library:** `mvdr_beamformer`
   - **Name:** `covariance_estimation` / `givens_qr_top` (match the top
     function name — this becomes the IP name in the Vivado catalog)
   - **Version:** `1.0`
   - **Display name / Description:** something human-readable, e.g.
     "MVDR Sample Covariance Matrix Estimator (M=64, K=129)" /
     "MVDR Givens-QR + Back-Substitution Solver (M=64)"
3. **RTL language:** Verilog (or VHDL — be consistent for both IPs to avoid
   mixed-language sim headaches later).
4. **Output:** leave the default export location, typically:
   ```
   <component_dir>/<solution>/impl/ip/
   ```
   You'll point Vivado's IP repository at this folder (or a copy of it).
5. Click **Run** / **Finish**. Vitis HLS produces a fully-formed IP-XACT
   package (`component.xml` + RTL sources + drivers).

### 3.2 Tcl method (for scripting/CI)

From the Vitis HLS Tcl console or a script:

```tcl
open_project cov_est_hls
open_solution "solution1"
set_top covariance_estimation
add_files covariance_matrix.cpp
add_files -tb tb_covariance_matrix.cpp
set_part {xcvc1902-vsva2197-2MP-e-S}
create_clock -period 3.33
csim_design
csynth_design
cosim_design
export_design -rtl verilog -format ip_catalog -display_name "MVDR Covariance Estimator" -description "MVDR sample covariance matrix kernel, M_ANT=64, K_SAMP=129" -vendor "mycompany" -library "mvdr_beamformer" -version "1.0" -ipname "covariance_estimation"
```

Repeat with the equivalent settings for `givens_qr_hls` / `givens_qr_top`.

### 3.3 Collect the IPs

Copy (or symlink) both exported `ip/` directories into a common repository
folder so Vivado can index them together, e.g.:

```
ip_repo/
├── covariance_estimation_1.0/
│   └── component.xml ...
└── givens_qr_top_1.0/
    └── component.xml ...
```

---

## 4. Create the Vivado project

1. **Vivado → Create Project → RTL Project** (or "Project is an extension
   of an HLS project" if you'd rather start from one of the HLS
   components — either works; the steps below assume a standalone Vivado
   project).
2. **Default part / board:** select `xcvc1902-vsva2197-2MP-e-S`, or the
   VCK190 board file if you have it installed (recommended — it pre-wires
   the CIPS + NoC + DDR memory controller defaults).
3. Finish project creation (no sources needed yet).

### 4.1 Register the IP repository

1. **Tools → Settings → IP → Repository** (or **Project Settings → IP**).
2. Click **+**, browse to `ip_repo/`, click **Select**.
3. Vivado scans and lists both IPs:
   - `MVDR Covariance Estimator` (`covariance_estimation_1.0`)
   - `MVDR Givens-QR Solver` (`givens_qr_top_1.0`)
4. Click **OK** — both now appear in the IP Catalog under
   **User Repository**.

---

## 5. Build the block design

**Tools → Create Block Design** (e.g. name it `mvdr_bd`).

### 5.1 Add the Versal CIPS and NoC

1. **+ (Add IP)** → search **Control, Interface and Processing System**
   (`versal_cips`) → add it.
2. Run **Run Block Automation** — Vivado will configure CIPS for the
   selected board (PMC, PS, NoC, DDR controller as applicable).
3. In the CIPS customization, under **PS-PL Interfaces**, enable at least
   one **AXI4 Master from PS (FPD/LPD)** for AXI4-Lite control of both
   kernels, e.g. `M_AXI_FPD` or `M_AXI_LPD`.
4. Add an **AXI NoC** (`axi_noc`) IP if not auto-added by board automation —
   this is how the `m_axi` ports of `covariance_estimation` (and possibly
   `givens_qr_top`, depending on §0 item 5) reach DDR.

### 5.2 Add `covariance_estimation`

1. **+ (Add IP)** → "MVDR Covariance Estimator" → add to canvas.
2. Run **Run Connection Automation** on its `s_axilite` (ctrl) port — Vivado
   proposes connecting it through an AXI Interconnect/SmartConnect to the
   PS master AXI port. Accept.
3. For the two `m_axi` ports (`gmem0` for `X`, `gmem1` for `R`):
   - Run **Connection Automation** on each — select **AXI NoC** as the
     destination. Vivado inserts an `axi_noc` slave interface (or routes
     into the existing one) and creates a memory-mapped connection to DDR.
   - Keep `gmem0` and `gmem1` on **separate NoC slave ports** if possible —
     this preserves the "independent NoC paths, zero arbitration" design
     intent noted in the kernel's comments.

### 5.3 Add `givens_qr_top`

Add it the same way. The wiring depends on what §2.2 step 7 found:

**Case A — `Lt`, `L`, `a_s`, `w_out` are `m_axi` + `s_axilite`:**
- Connect `s_axilite` to the same AXI Interconnect/SmartConnect used for
  `covariance_estimation`'s control port (sharing one PS master AXI is fine
  for control traffic).
- Connect each `m_axi` port to the AXI NoC (same as §5.2), giving
  `givens_qr_top` its own DDR-mapped buffers for `Lt`, `L`, `a_s`, `w_out`.
- The Cholesky stage (§9) reads `R` from DDR and writes `L`/`Lt` to the
  addresses `givens_qr_top` will read from — fully decoupled via memory,
  PS-orchestrated.

**Case B — `Lt`, `L`, `a_s`, `w_out` are `ap_memory`/`bram`-style ports:**
- These appear as separate address/data/enable buses (one set per array,
  per-array `_address0`, `_ce0`, `_we0`, `_d0`, `_q0`, etc.).
- They are **not** meant to go to the NoC. Instead they connect directly to
  Block Memory Generator / Dual-Port BRAM IPs (one BRAM per array, sized to
  match `M×M` or `M` elements at the relevant bit width), or — if a Cholesky
  HLS IP was built with matching `ap_memory` outputs — directly
  port-to-port between the Cholesky IP's output BRAM interfaces and
  `givens_qr_top`'s input BRAM interfaces (true dataflow chaining, no DDR
  round-trip).
- Still add `s_axilite` (control: `ap_start`/`ap_done`/`ap_idle`) to the PS
  AXI Interconnect so software can kick off and poll the kernel.

> If you're unsure which case applies, re-check the **Interface** tab of the
> C-synthesis report for `givens_qr_top` in Vitis HLS (§2.2 step 7) — it
> lists every port and its resolved interface type explicitly.

### 5.4 Clocking and reset

1. All three blocks (CIPS/NoC, `covariance_estimation`,
   `givens_qr_top`) must run from the **same 300 MHz clock domain** the
   kernels were synthesized for (`create_clock -period 3.33`).
2. If CIPS doesn't directly provide a 300 MHz PL clock, add a
   **Clocking Wizard** IP to derive 300 MHz from the available reference
   clock, and connect its output to the `ap_clk` of both HLS IPs and to the
   AXI Interconnect/SmartConnect/NoC clock inputs.
3. Add a **Processor System Reset** IP, driven from CIPS's reset output and
   the Clocking Wizard's `locked` signal, and fan its `peripheral_aresetn`
   out to `ap_rst_n` on both HLS IPs and to the interconnect reset inputs.

### 5.5 Address map

1. Open the **Address Editor** tab.
2. Assign DDR-backed offsets/ranges for:
   - `covariance_estimation`: `X` (read buffer, `M_ANT*K_SAMP*sizeof(csamp_t)`
     ≈ 64×129×4 B ≈ 33 KB) and `R` (write buffer,
     `M_ANT*M_ANT*sizeof(ccov_result_t)` ≈ 64×64×8 B = 32 KB).
   - If Architecture A: `givens_qr_top`'s `Lt`, `L`, `a_s`, `w_out` buffers
     (sizes: `L`/`Lt` are `M×M` of `cplx_chol` = 16-bit×2 → 64×64×4 B = 16 KB
     each; `a_s`/`w_out` are `M` elements of their respective complex types
     — a few hundred bytes each).
   - Each kernel's `s_axilite` control register block (size reported by
     Vitis HLS, typically a few KB — Vivado fills this in automatically once
     connected).
3. Make sure no ranges overlap, and **record every base address** — you'll
   need them for the PS-side driver/software in §10.

---

## 6. Handling the missing Cholesky stage (the actual integration gap)

`covariance_estimation` produces `R` (`ccov_result_t[64][64]`).
`givens_qr_top` consumes `L`/`Lt` (`cplx_chol[64][64]`, the Cholesky factors
of `R`). **No kernel that performs `R → L, Lt` was provided.** Pick one:

**Architecture A — DDR-mediated, PS computes (or accelerates) Cholesky**
- `covariance_estimation` writes `R` to DDR.
- PS (Arm core in CIPS) reads `R`, runs a Cholesky decomposition (software,
  or calls a third accelerator/IP — e.g. an existing Cholesky HLS kernel if
  you have one elsewhere in your repo), writes `L`/`Lt` to the DDR addresses
  `givens_qr_top` is configured to read.
- Simplest to integrate into *this* block design (no new RTL needed right
  now), at the cost of PS round-trip latency between stages.

**Architecture B — Build a third HLS IP for Cholesky and chain in hardware**
- Write a `cholesky_decomp` HLS kernel: input `ccov_result_t R[64][64]`,
  outputs `cplx_chol L[64][64]`, `cplx_chol Lt[64][64]` (this is where the
  rounding-mode note in §0 item 4 matters — converting `cov_result_t`
  (32,8) down to `t_chol` (16,5) is a real, intentional requantization step;
  match its rounding mode to `cov_result_t`'s `AP_RND_CONV` for consistency
  with the upstream kernel's golden model).
- Export it as a third IP exactly as in §3, add it to the block design
  between the other two, and either:
  - chain it via DDR (same as Architecture A's wiring, but in hardware,
    PS only sequences `ap_start`/`ap_done` for all three kernels), or
  - if all three kernels expose `ap_memory`/`bram` ports (Case B in §5.3),
    wire BRAM-to-BRAM directly for a true dataflow pipeline with no DDR
    round-trips between stages.

Either way, **this is the piece to design/locate before final export** —
everything else in this guide (address map, connection automation, driver
sequencing) is the same regardless of which architecture you pick, just
with one extra IP instance for Architecture B.

---

## 7. Validate, simulate, and generate output

1. **Validate Design** (toolbar or **Tools → Validate Design**, shortcut
   `F6`). Resolve any DRC errors (unconnected required ports, clock/reset
   mismatches, address conflicts) before continuing.
2. **Create HDL Wrapper** (right-click the block design in Sources →
   *Create HDL Wrapper*, "Let Vivado manage wrapper and auto-update").
3. **Behavioral simulation** (optional but recommended): run
   **Run Simulation → Run Behavioral Simulation** on the block design with
   a small testbench that drives the PS-AXI side (or use the
   AXI Verification IP / a simple BFM) to issue the same stimulus as
   `tb_covariance_matrix.cpp` / `tb_givens_qr.cpp`, confirming end-to-end
   behavior matches the Vitis HLS co-simulation results.
4. **Run Synthesis → Run Implementation → Generate Bitstream.**
5. **File → Export → Export Hardware** (include bitstream) to produce an
   `.xsa` for the software side (Vitis IDE application project, baremetal
   or Linux).

---

## 8. Software-side sequencing (high-level)

Once the `.xsa` is imported into a Vitis platform/application project,
Vitis auto-generates drivers (`xcovariance_estimation.h`,
`xgivens_qr_top.h`, and a Cholesky driver if applicable) with register
offsets matching the address map from §5.5. The control flow is:

1. DMA/copy snapshot data `X` into the `covariance_estimation` input buffer
   address.
2. Write base addresses to `covariance_estimation`'s control registers
   (`XCovariance_estimation_Set_X`, `..._Set_R`, generated by Vitis), set
   `ap_start`, poll `ap_done`.
3. (Architecture A) Read `R`, run/launch Cholesky, write `L`/`Lt` to
   `givens_qr_top`'s input addresses.
   (Architecture B) Set `cholesky` IP's base addresses, start, poll done.
4. Write base addresses to `givens_qr_top`'s control registers
   (`L`, `Lt`, `a_s`, `w_out`), set `ap_start`, poll `ap_done`.
5. Read `w_out` from its output buffer — this is the unnormalized MVDR
   weight vector.

---

## 9. Final pre-export checklist

- [ ] `givens_qr.h`: `#define M 64` (was 8)
- [ ] `givens_qr.h`: `t_accum` widened to ≥12 integer bits (per FIX-3)
- [ ] `tb_givens_qr.cpp`: any hardcoded `M=8` loop bounds/reference arrays
      updated to `M=64`
- [ ] `givens_qr.cpp`: interface pragmas confirmed for `Lt`, `L`, `a_s`,
      `w_out`, `return` (Case A vs Case B determines Vivado wiring — see §5.3)
- [ ] Cholesky stage decided (Architecture A or B, §6) and, if B, that
      kernel written/validated/exported as a third IP
- [ ] If `cplx_rxx`/`t_rxx` end up used anywhere (e.g., a Cholesky kernel's
      input), rounding mode aligned with `cov_result_t` (`AP_RND_CONV`)
- [ ] Both kernels: C-sim, C-synth, and **C/RTL co-sim all pass** for `M=64`
      before exporting RTL
- [ ] Both kernels exported to the same `ip_repo/` with consistent
      vendor/library naming
- [ ] Block design clocked at 300 MHz with shared reset; address map has no
      overlaps; addresses recorded for software
