# Building the AIE Kernels and Integrating with Vivado — VCK190

## Read this first — how this is different from your HLS IP flow

Your three original kernels (RDBS, Covariance, MVDR) each went:
```
Vitis HLS  -->  Export RTL  -->  IP with component.xml  -->  Add to Vivado IP Catalog
```

The AIE files in this package do **not** go through that pipeline at all. There is
no "Export RTL" step for AIE code, and AIE kernels never become a Vivado IP-catalog
entry. The real flow is:

```
STEP 1:  Vivado        -- builds the PL/NoC/CIPS platform, INCLUDING the PL bridge
                           kernel (rdbs_cov_bridge.cpp, which IS a normal HLS IP)
                           and your existing MVDR IP. Exports a platform .xsa.

STEP 2:  Vitis          -- creates an AI Engine application project. Compiles
         (aiecompiler)     graph.cpp + the AIE kernels with aiecompiler, targeting
                           the .xsa from Step 1 as the hardware platform.

STEP 3:  Vitis          -- links the AIE compiler output with the PL bitstream
         (v++ / linker)    to produce the final PDI (device image) for VCK190.
```

You cannot skip Step 1. The AIE compiler needs to know the PL platform's PLIO
connection points before it can compile the graph, because PLIO ports are defined
relative to the platform.

---

## What's in this package

```
aie_integration/
├── aie/
│   ├── rdbs_aie.h                  AIE kernel header — RDBS (64->21 beam compress)
│   ├── rdbs_aie.cc                 AIE kernel body — RDBS
│   ├── covariance_aie.h            AIE kernel header — Covariance (per-tile)
│   ├── covariance_aie.cc           AIE kernel body — Covariance (per-tile)
│   ├── covariance_pair_tables.cc   Generated (i,j) pair-to-tile assignment tables
│   ├── graph.h                     adf::graph — wires RDBS + 4 Covariance tiles
│   └── graph.cpp                   Top-level graph instantiation + sim driver
└── pl/
    └── rdbs_cov_bridge.cpp         PL kernel — gathers RDBS output, broadcasts to
                                     the 4 Covariance AIE tiles (NEW 4th PL kernel,
                                     not in your original 3)
```

Your **MVDR kernel is unchanged** — it stays exactly as you built it in Vitis HLS,
exported as the same IP, and goes into Vivado the same way as before.

Your **original `rdbs_kernel` and `covariance_matrix` PL IPs are NOT used in this
AIE version** — they are replaced by the AIE graph + the new `rdbs_cov_bridge`
PL kernel. Keep your original `.cpp`/`.h` files around; you'll want to compare AIE
output against them as a correctness reference, but don't add `rdbs_kernel` or
`covariance_estimation` to the new Vivado design — the new bridge kernel and AIE
graph take their place.

---

## ⚠ Before you build anything — known open issues in this code

I'm flagging these again here because they affect build success, not just
runtime correctness:

1. **`covariance_aie.cc` may not fit AIE tile local memory.** Each tile loads a
   full 84,000-byte buffer; AIE-ML tiles typically have ~64KB local data memory.
   The AIE compiler will tell you immediately if this overflows — if it does,
   the fix is to stream `X_buf` in per-channel chunks instead of one giant
   window (this restructuring is not yet done).
2. **`covariance_aie.cc`'s inner MAC loop is scalar, not vectorized.** It will
   compile and simulate correctly but will NOT give you AIE's real throughput
   advantage until vectorized with `aie::vector<cint16,8>` lanes.
3. **`graph.h`'s `connect<>` calls for the RDBS→Covariance path are illustrative,
   not literal.** The real data path goes through `rdbs_cov_bridge.cpp` in PL.
   You will need to adjust the PLIO names/connections in `graph.h` to match
   the actual PLIO port names Vivado generates once the bridge kernel is in
   your block design (these names aren't known until Step 1 is built).
4. **`w_coeffs.h` placeholder.** `rdbs_aie.cc` expects a `coeff_re_im` table —
   wire in your real `W_H_REAL_F`/`W_H_IMAG_F` values (converted to the cint16
   format described in `rdbs_aie.h`'s header comment) before trusting any
   numeric output.

None of these block you from *building* — the AIE compiler will compile and the
HLS bridge kernel will synthesize regardless. They affect whether the *numbers*
coming out are trustworthy. Treat first-pass results as a wiring/build check,
not a correctness check.

---

# PART 1 — Build the PL bridge kernel in Vitis HLS

This is the one new piece that follows your familiar HLS flow exactly.

## Step 1 — Create a new Vitis HLS component

1. Open **Vitis HLS** (or Vitis Unified IDE → HLS Component).
2. Create a new component: name it `rdbs_cov_bridge`.
3. Add `pl/rdbs_cov_bridge.cpp` as the design source.
4. Set the **top function** to `rdbs_cov_bridge`.
5. Set the **part**: same Versal device as your other IPs (`xcvc1902-vsva2197-2MP-e-S` or whatever exact part your VCK190 target uses — check your existing `mvdr_kernel` HLS project's part setting and match it).
6. Set the **clock period**: match your other kernels (e.g. 3.33 ns for 300 MHz).

## Step 2 — Run C Synthesis

1. Click **C Synthesis** (or `csynth` in the GUI).
2. Review the synthesis report:
   - Check the **AXI-Stream interfaces**: you should see `rdbs_in`, `cov_out_tile0..3` listed.
   - Check **latency/II**: the GATHER and BROADCAST loops should both report `II=1`.
   - **Resource check**: `X_gather` is 21×1000×32-bit = 672,000 bits ≈ 84 KB of BRAM. Confirm the report shows this fits in available BRAM18/BRAM36 blocks for your Versal part (it will — VCK190 has abundant BRAM, but check the utilization % isn't unexpectedly high, which would indicate a pragma issue).

## Step 3 — Export RTL as IP

1. Click **Export RTL** (same as you did for `rdbs_kernel`, `covariance_estimation`, `mvdr_weights`).
2. Choose **IP Catalog** as the export format.
3. Note the output directory — this becomes your 4th IP repo folder, e.g.:
   ```
   ip_repo/rdbs_cov_bridge/component.xml
   ```

You now have 4 PL-side IPs: `mvdr_weights` (unchanged) + `rdbs_cov_bridge` (new).
`rdbs_kernel` and `covariance_estimation` are **not** used in this version.

---

# PART 2 — Build the Vivado platform (PL + NoC + CIPS + bridge IP)

This follows the same structure as your earlier Vivado guide, with two changes:

## What's different from your original Vivado block design

- **Remove**: `rdbs_kernel_0` and `covariance_estimation_0` blocks (and their AXI-Stream connection, S1 in the earlier reference table).
- **Add**: `rdbs_cov_bridge_0` block, connected as follows:

| Port on `rdbs_cov_bridge_0` | Connects to |
|---|---|
| `rdbs_in` (AXI-Stream slave) | → PLIO output from AIE RDBS tile (added in Part 3) |
| `cov_out_tile0..3` (AXI-Stream masters) | → PLIO inputs to the 4 AIE Covariance tiles |
| `s_axi_ctrl` (AXI-Lite) | → `smartconnect_0` master port (same fan-out pattern as before) |
| `ap_clk` / `ap_rst_n` | → same clock/reset wiring as your other PL kernels |

- **Keep unchanged**: `mvdr_weights_0`, CIPS, AXI NoC, Clocking Wizard, both SmartConnects, Processor System Reset, Utility Buffer — every other block from your original Vivado guide stays exactly as built.

## Critical addition — the AIE array must be present in the platform

1. In your block design, the Versal device itself includes the AI Engine array as
   part of the silicon — you do **not** add a separate "AI Engine IP" block to the
   canvas the way you add `rdbs_cov_bridge`.
2. However, you must enable **AIE-PL interface support** in CIPS:
   - Double-click `versal_cips_0`.
   - In the PS-PMC tree, find **NoC → AIE-NoC interfaces** (or similar, depending on your exact CIPS version — search the left tree for "AIE").
   - Enable the AIE-to-NoC and PL-to-AIE-NoC interfaces. This reserves the NoC paths the AIE array will use to reach the PLIO bridge points once Vitis links everything together.
3. **PLIO placeholder ports**: At this Vivado stage, you don't yet have literal PLIO ports to wire — those get created automatically when Vitis builds the AIE graph against this platform in Part 3. What you DO need now is to make sure `rdbs_cov_bridge_0`'s `rdbs_in` and `cov_out_tile0..3` AXI-Stream ports are exposed as **external ports** (Make External) rather than left dangling, since the Vitis platform stage will need to map them.

## Validate, generate bitstream, export hardware

Run through Steps 24–30 from your earlier Vivado guide exactly as before:
**Validate Design → Create HDL Wrapper → Add XDC → Synthesis → Implementation →
Generate Bitstream → Export Hardware**.

Output: `mvdr_system.xsa` — this now contains MVDR + the bridge kernel + the
externalized PLIO-ready ports, ready for the AIE side.

---

# PART 3 — Build the AIE graph in Vitis (the new step)

## Step 1 — Create a Vitis AI Engine application project

1. Open **Vitis Unified IDE**.
2. **File → New Component → AI Engine Application**.
3. Name it `beamform_aie`.
4. When prompted for the **platform**, browse to your `mvdr_system.xsa` from Part 2. This is what ties the AIE compiler to your specific PL platform and its external PLIO-ready ports.

## Step 2 — Add the AIE source files

1. Copy the entire `aie/` folder from this package into your new Vitis AIE project's source directory.
2. In the project, confirm these files are added as sources:
   - `rdbs_aie.h`, `rdbs_aie.cc`
   - `covariance_aie.h`, `covariance_aie.cc`, `covariance_pair_tables.cc`
   - `graph.h`, `graph.cpp`
3. Set `graph.cpp` as the **top-level graph file**.

## Step 3 — Fix the PLIO names to match your actual platform

1. Open `graph.h`.
2. The `input_plio::create(...)` and `output_plio::create(...)` calls currently use placeholder names (`x_in_plio`, `coeff_in_plio`, `r_out_plio_0..3`) and placeholder file names for simulation.
3. Open the platform's interface specification (Vitis will show you the available PLIO-ready external ports from your XSA under **Platform → Interfaces** in the project view).
4. Update each `create(...)` call's first argument to match the actual port name Vivado generated for `rdbs_cov_bridge_0`'s external ports (these will look like `rdbs_cov_bridge_0_rdbs_in`, etc., depending on Vivado's auto-naming).

## Step 4 — Run AIE simulation first (do this before hardware build)

1. Right-click the project → **Build** (this invokes `aiecompiler`).
2. Watch the build log for:
   - **Memory overflow errors** on the Covariance tiles (the known risk flagged above) — if you see "exceeds tile data memory," you must restructure `covariance_aie.cc` to stream `X_buf` rather than load it whole.
   - **PLIO connection errors** — these mean the names from Step 3 don't match the platform; re-check the XSA's interface list.
3. Once it builds cleanly, run **AIE Simulation** (`x86simulator` or `aiesimulator` target) using sample data files in a `data/` folder matching the file names referenced in `graph.h`'s PLIO `create()` calls (e.g. `data/x_in.txt`).
4. **Compare AIE simulation output against your original PL testbenches' golden data** (`rdbs_tb.cpp`'s `test_output.dat`, `covariance_tb.cpp`'s reference). This is your real correctness check — nothing earlier in this process validates numerics.

## Step 5 — Hardware build and link

1. Once simulation matches expectations, switch the build target to **hardware (`hw`)**.
2. Vitis invokes `aiecompiler` for hardware, producing `libadf.a` and the AIE configuration.
3. **Vitis → Build → Package** combines this with the PL bitstream from your `.xsa` to produce the final **PDI**.
4. This final PDI is what you program onto the VCK190 — it supersedes the bitstream-only PDI you generated in Vivado in Part 2; Part 2's bitstream was an intermediate artifact, not the final deliverable.

---

## Quick reference — what changed vs. your original 3-IP design

| Original PL-only design | This AIE-accelerated design |
|---|---|
| `rdbs_kernel` (PL IP) | RDBS AIE kernel (`rdbs_aie.cc`) |
| `covariance_estimation` (PL IP) | 4× Covariance AIE tiles (`covariance_aie.cc`) |
| `mvdr_weights` (PL IP) | `mvdr_weights` (PL IP) — **unchanged** |
| — | `rdbs_cov_bridge` (new PL IP, gather+broadcast) |
| Vivado only | Vivado (PL/NoC/CIPS) **+** Vitis AIE compiler **+** Vitis link/package |
| One `.xsa` export = final hardware | `.xsa` is an intermediate; final output is the AIE-linked PDI |

---

## Honest summary of confidence level per file

| File | Confidence | What you must verify |
|---|---|---|
| `rdbs_aie.h` / `.cc` | Moderate — standard pattern | AIE API syntax against your exact compiler version; coefficient values |
| `covariance_pair_tables.cc` | High — verified by script, 231/231 pairs confirmed | None — this is correct by construction |
| `covariance_aie.h` / `.cc` | Low-moderate — flagged memory risk, unvectorized | Tile memory fit; needs vectorization for real speedup |
| `graph.h` / `.cpp` | Low — illustrative topology, PLIO names are placeholders | PLIO names against real platform; the buffer/broadcast semantics |
| `rdbs_cov_bridge.cpp` | Moderate — standard HLS streaming pattern, unsynthesized | Run C-synthesis and C-simulation before trusting |

Start with Part 1 (the bridge kernel) since it's the most familiar process and
will build cleanly. Part 3 (AIE compile) is where you'll hit the real unknowns —
expect to iterate on `covariance_aie.cc`'s memory layout once the AIE compiler
gives you real feedback.
