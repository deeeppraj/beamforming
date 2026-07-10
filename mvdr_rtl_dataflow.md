# MVDR Beamformer Block Design — RTL Architecture & Data Flow
## Prepared for Mentor Review
### Platform: Xilinx Versal VCK190 | Tool: Vivado/Vitis HLS 2025.2

---

## 1. System Overview

This design implements a hardware-accelerated **Minimum Variance Distortionless
Response (MVDR) adaptive beamformer** on the Xilinx Versal VCK190 evaluation
platform. The algorithm compresses a 64-element antenna array down to 21
beamspace channels, estimates a covariance matrix from 1000 snapshots of
received data, and solves for the optimal weight vector that maximises
signal-to-interference-plus-noise ratio (SINR) in a chosen look direction.

The entire signal processing chain runs in Programmable Logic (PL), controlled
by the Arm Cortex-A72 Application Processor Unit (APU) in the Processing System
(PS). Data moves between PS and PL via the AXI Network-on-Chip (NoC), which
also provides the interface to the DDR4 external memory.

### System-Level Block Diagram

```
╔══════════════════════════════════════════════════════════════════════════╗
║  VERSAL VCK190                                                           ║
║                                                                          ║
║  ┌─────────────────────────────────────────────────────────────────┐    ║
║  │  PROCESSING SYSTEM (PS)                                          │    ║
║  │                                                                   │    ║
║  │  ┌──────────────┐   AXI4   ┌──────────────────────────────────┐ │    ║
║  │  │ Cortex-A72   │─────────▶│        AXI NoC                   │ │    ║
║  │  │ (main.c)     │ FPD_CCI  │                                   │ │    ║
║  │  │              │◀─────────│  S00─S03: FPD_CCI (coherent)     │ │    ║
║  │  │              │  M_AXI   │  S04:     LPD                    │ │    ║
║  │  │              │─────────▶│  S05:     PMC                    │ │    ║
║  │  │  (HLS kernel │  FPD     │  S06:     DMA MM2S               │ │    ║
║  │  │   control    │          │  S07:     DMA S2MM (unused)      │ │    ║
║  │  │   via AXI-   │          │  S08:     Covariance gmem        │ │    ║
║  │  │   Lite regs) │          │  S09:     MVDR SmartConnect      │ │    ║
║  │  └──────────────┘          │                                   │ │    ║
║  │                            │       MC (Memory Controller)      │ │    ║
║  │                            └───────────────┬──────────────────┘ │    ║
║  └────────────────────────────────────────────┼────────────────────┘    ║
║                                               │                          ║
║                                          DDR4 DIMM                       ║
║                                         (8 GB, 64-bit)                   ║
║                                                                          ║
║  ┌────────────────────────────────────────────────────────────────────┐  ║
║  │  PROGRAMMABLE LOGIC (PL)                                           │  ║
║  │                                                                     │  ║
║  │  ┌─────────┐  AXIS  ┌──────────┐  AXIS  ┌────────────┐            │  ║
║  │  │ AXI DMA │───────▶│  RDBS    │───────▶│ Covariance │            │  ║
║  │  │         │ 64-bit │  kernel  │ 64-bit │ estimation │            │  ║
║  │  │         │        │          │        │            │            │  ║
║  │  │ (reads  │        │ 64×1000  │        │ 21×1000    │──AXI4────▶ │  ║
║  │  │ input   │        │ → 21×1000│        │ → R[21×21] │  to DDR    │  ║
║  │  │ from    │        │ beamspace│        │ Hermitian  │            │  ║
║  │  │ DDR)    │        │ compress │        │ matrix     │            │  ║
║  │  └─────────┘        └──────────┘        └────────────┘            │  ║
║  │       ▲                                                             │  ║
║  │       │ AXI-Lite control (start/done/addr)                         │  ║
║  │  ┌────┴──────────┐                        ┌────────────┐           │  ║
║  │  │ SmartConnect  │                        │    MVDR    │           │  ║
║  │  │   (control)   │───────────────────────▶│   weights  │──AXI4───▶ │  ║
║  │  │  SC_0         │                        │            │  to DDR   │  ║
║  │  │ 1S → 4M       │                        │ R,a → w    │           │  ║
║  │  └───────────────┘                        └────────────┘           │  ║
║  │                                                                     │  ║
║  │  ┌──────────────────┐   ┌─────────────────┐                        │  ║
║  │  │  Clocking Wizard │   │ Proc Sys Reset  │                        │  ║
║  │  │  100→300 MHz     │   │                 │                        │  ║
║  │  └──────────────────┘   └─────────────────┘                        │  ║
║  └────────────────────────────────────────────────────────────────────┘  ║
╚══════════════════════════════════════════════════════════════════════════╝
```

---

## 2. Clock and Reset Architecture

### 2.1 Clock Domains

The design uses two clock domains, both ultimately sourced from CIPS.

```
CIPS (PMC PLL)
    │
    └─▶ pl0_ref_clk (100 MHz)
              │
              └─▶ Clocking Wizard (MMCME5)
                        │
                        └─▶ clk_out1 (300 MHz) ──▶ All PL logic:
                                                     • AXI DMA
                                                     • rdbs_kernel
                                                     • covariance_estimation
                                                     • mvdr_weights
                                                     • SmartConnect (both)
                                                     • NoC aclk6 (PL slaves)

CIPS (internal APU/FPD PLL)
    ├─▶ fpd_cci_noc_axi0_clk ──▶ NoC aclk0 (FPD CCI coherent path)
    ├─▶ fpd_cci_noc_axi1_clk ──▶ NoC aclk1
    ├─▶ fpd_cci_noc_axi2_clk ──▶ NoC aclk2
    ├─▶ fpd_cci_noc_axi3_clk ──▶ NoC aclk3
    ├─▶ lpd_axi_noc_clk      ──▶ NoC aclk4 (LPD path)
    ├─▶ pmc_axi_noc_axi0_clk ──▶ NoC aclk5 (PMC path)
    └─▶ m_axi_fpd_aclk       ──▶ SmartConnect_0 aclk (control path)
```

The MMCME5 inside Clocking Wizard is configured with:
- Input: 100 MHz (no buffer, connects directly to CIPS pl0_ref_clk)
- CLKFBOUT_MULT = 12, DIVCLK_DIVIDE = 1, CLKOUT0_DIVIDE = 4
- Output: 100 × 12 / 1 / 4 = **300 MHz** for all three HLS kernels

All measured timing results showed WNS = +3.272 ns and WHS = +0.029 ns at
300 MHz, confirming the clock tree is cleanly closed with comfortable margin.

### 2.2 Reset Domain

```
CIPS
  └─▶ pl0_resetn (active-low, released after PL configuration)
            │
            └─▶ Processor System Reset (proc_sys_reset_0)
                    ├─▶ Input: ext_reset_in  ← pl0_resetn
                    ├─▶ Input: dcm_locked    ← clk_wizard_0/locked
                    ├─▶ Input: slowest_sync_clk ← 300 MHz
                    │
                    └─▶ peripheral_aresetn (active-low, synchronised to 300 MHz)
                              │
                              ├─▶ rdbs_kernel_0/ap_rst_n
                              ├─▶ covariance_estimation_0/ap_rst_n
                              ├─▶ mvdr_weights_0/ap_rst_n
                              ├─▶ axi_dma_0/axi_resetn
                              ├─▶ smartconnect_0/aresetn
                              └─▶ smartconnect_mem_0/aresetn
```

The proc_sys_reset IP holds all PL logic in reset until two conditions are
both true simultaneously: (1) pl0_resetn has been deasserted by the PMC after
completing configuration, and (2) the Clocking Wizard's locked signal has gone
high, indicating the 300 MHz output PLL has locked and is stable. This prevents
any HLS kernel from starting in an unknown state due to an unstable clock.

---

## 3. Memory Map

Before tracing data flow, it is essential to understand where everything lives
in the 64-bit address space as seen by the Cortex-A72.

```
Address                  Size    Contents
─────────────────────────────────────────────────────────────────────────
0x0000_0000_0000_0000    2 GB    DDR4 — general purpose (NoC DDR_LOW0)
0x0000_0008_0000_0000    6 GB    DDR4 — extended (NoC DDR_LOW1)

0x0000_00A4_0000_0000    64 KB   rdbs_kernel    AXI-Lite control registers
0x0000_00A4_0001_0000    64 KB   covariance_estimation AXI-Lite registers
0x0000_00A4_0002_0000    64 KB   mvdr_weights   AXI-Lite control registers
0x0000_00A4_0003_0000    64 KB   axi_dma_0      AXI-Lite control registers

Within DDR, main.c partitions the space:
0x0000_0000_1000_0000    512 KB  input_samples  (64000 × 8 bytes, 64-bit words)
0x0000_0000_1010_0000    3.5 KB  R matrix       (21×21 × 8 bytes = 3528 bytes)
0x0000_0000_1020_0000    168 B   steering vec a (21 × 8 bytes)
0x0000_0000_1030_0000    168 B   output weights w (21 × 8 bytes)
```

All four DDR sub-regions sit safely apart with 1 MB spacing, preventing any
burst-write overrun from one region corrupting another.

---

## 4. Control Path — How Software Starts the Hardware

The PS uses the M_AXI_FPD master port (the FPD's 64-bit general-purpose AXI
master) to reach the AXI-Lite control register blocks of all four controllable
peripherals (three HLS kernels + DMA). The path is:

```
Cortex-A72 (AXI4 master, cache coherent through FPD CCI)
    │
    └─▶ CIPS M_AXI_FPD (AXI4, 128-bit wide, FPD clock domain)
              │
              └─▶ SmartConnect_0 (1 slave, 4 masters, clock-domain crossing
              │                    between FPD clock and 300 MHz implicitly
              │                    handled by SmartConnect's internal FIFO)
              │
              ├─▶ M00_AXI ──▶ rdbs_kernel_0/s_axi_control    (0xA400_0000)
              ├─▶ M01_AXI ──▶ covariance_estimation_0/s_axi_control (0xA401_0000)
              ├─▶ M02_AXI ──▶ mvdr_weights_0/s_axi_control   (0xA402_0000)
              └─▶ M03_AXI ──▶ axi_dma_0/S_AXI_LITE           (0xA403_0000)
```

Each HLS kernel's AXI-Lite register block (generated by Vitis HLS) exposes
the following at fixed offsets from its base address:

```
Offset  Register   Bits  Description
──────────────────────────────────────────────────────────
0x00    CTRL        [0]   ap_start — write 1 to start the kernel
                   [1]   ap_done  — reads 1 when kernel has finished
                   [2]   ap_idle  — reads 1 when kernel is idle (waiting)
                   [3]   ap_ready — reads 1 when kernel can accept new start
                   [7]   auto_restart
0x04    GIER        [0]   Global interrupt enable
0x08    IP_IER      [0]   ap_done interrupt enable
                   [1]   ap_ready interrupt enable
0x0C    IP_ISR      [0]   ap_done interrupt status
                   [1]   ap_ready interrupt status
```

For kernels that take DDR buffer pointers as arguments (covariance_estimation
and mvdr_weights), additional registers at higher offsets hold the low and high
32 bits of the 64-bit DDR address. For rdbs_kernel, there is a single scalar
argument register at offset 0x10 holding num_snapshots.

In `main.c`, the driver calls `XRdbs_kernel_Set_num_snapshots(&inst, 1000)`
and `XRdbs_kernel_Start(&inst)` which perform AXI-Lite register writes — a
32-bit write of 0x000003E8 to offset 0x10 (num_snapshots=1000) followed by
a 32-bit write of 0x01 to offset 0x00 (ap_start=1).

---

## 5. Data Path — End to End

The actual signal processing data travels a completely separate path from the
control registers. The following describes each hop in the order data flows
during a beamforming frame.

### 5.1 Stage 0: PS writes input data to DDR

```
Cortex-A72 executes memcpy(DDR_INPUT_BASE, input_samples, 512000)
    │
    ├─▶ Write transactions over FPD_CCI_NOC_0..3 (cache coherent, 128-bit wide)
    │       ──▶ NoC S00..S03 ──▶ MC Port 0..3 ──▶ DDR4
    │
    └─▶ Xil_DCacheFlushRange() — ensures all cache lines are written back
        to DDR before the DMA is allowed to read them
```

The 512000 bytes of packed input (64000 samples × 8 bytes each, with 32-bit
packed real/imag in the low half and zero in the upper half) are written by
the A72 as a sequence of AXI4 burst write transactions through the NoC's
FPD CCI slaves, which are connected to all four DDR memory controller ports
in an interleaved fashion to distribute the load.

At the same time, the steering vector (168 bytes) is written to 0x10200000
via the same path.

The cache flush is critical: without it, the DDR may still contain stale data
(the A72's L1/L2 caches would hold the newly written data, but the DMA engine
is not cache-coherent — it reads directly from DDR via the NoC, bypassing all
caches). The flush forces every dirty cache line to be written back.

### 5.2 Stage 1: AXI DMA reads from DDR and streams to RDBS

```
A72 writes DMA control registers via AXI-Lite (SmartConnect_0 M03):
  SA = 0x10000000   (source address in DDR)
  Length = 512000   (bytes)
  DMACR.RS = 1      (run/start)

DMA MM2S channel:
  ├─▶ Issues AXI4 read bursts on M_AXI_MM2S
  │     • 64-bit wide (matches NoC S06 and rdbs_kernel x_in TDATA)
  │     • Up to 256-beat bursts (256 × 8 = 2048 bytes per transaction)
  │     • Max outstanding: controlled by DMA internals
  │
  ├─▶ M_AXI_MM2S ──▶ NoC S06_AXI ──▶ DDR reads (data comes back)
  │
  └─▶ DMA internal MM2S FIFO / datapath
        │
        └─▶ M_AXIS_MM2S (AXI4-Stream, TDATA=64-bit, TVALID/TREADY handshake)
                  │
                  └─▶ rdbs_kernel_0/x_in
```

The DMA generates 512000 / 8 = **64000 AXI-Stream beats** total, which
precisely matches the number of reads the RDBS kernel's READ_INPUT loop
will execute (1000 snapshots × 64 channels = 64000).

Each beat on the wire:
- TDATA[31:16] = real component, ap_fixed<16,1> bit pattern
- TDATA[15:0]  = imag component, ap_fixed<16,1> bit pattern
- TDATA[63:32] = 0 (upper half; RDBS kernel ignores it — only data[31:0]
                   is consumed in rdbs_kernel.cpp's READ_INPUT loop)
- TVALID       = 1 when DMA has data ready
- TREADY       = 1 when RDBS is ready to consume (backpressure mechanism)

The TVALID/TREADY handshake is the fundamental AXI4-Stream flow control. If
RDBS is busy computing (in BEAM_LOOP) and not reading from x_in, TREADY goes
low, and the DMA stalls — it stops issuing DDR read requests until TREADY
rises again. This prevents data loss without any software intervention.

Note: `HAS_TLAST=0` on rdbs_kernel_0/x_in (confirmed from component.xml).
The kernel does not use TLAST on its input port at all — it counts beats via
its internal loop counter (M_ELEMENTS × num_snapshots) and terminates
naturally, regardless of any TLAST signaling on the wire.

### 5.3 Stage 2: RDBS kernel — Beamspace compression

The RDBS (Reduced-Dimension Beamspace) kernel performs a matrix-vector
multiply on each snapshot: it projects the 64-element spatial sample vector
down to a 21-element beamspace vector using a pre-stored 21×64 complex
weight matrix W^H (loaded once from ROM at first invocation).

```
Internal data flow (from rdbs_kernel.cpp):

SNAPSHOT_LOOP (k = 0..999):
│
├─▶ READ_INPUT (m = 0..63):  [pipelined, II=1]
│     For each of 64 channels per snapshot:
│       Read one 64-bit beat from x_in AXI-Stream
│       Bit-extract: xr.range(15,0) = pkt.data.range(31,16)  ← real
│                    xi.range(15,0) = pkt.data.range(15,0)   ← imag
│       Store in local buffers x_real[64], x_imag[64]
│       (both ARRAY_PARTITION complete → 64 separate registers,
│        all readable in parallel in the next phase)
│
└─▶ BEAM_LOOP (b = 0..20):   [pipelined, II=1]
      For each of 21 output beams:
        DOT_PRODUCT (m = 0..63):  [UNROLLED — 64 parallel MACs]
          acc_real += w_real[b][m] * x_real[m] - w_imag[b][m] * x_imag[m]
          acc_imag += w_real[b][m] * x_imag[m] + w_imag[b][m] * x_real[m]
        Truncate to ap_fixed<16,4> (range [-8, +8), 12 fractional bits)
        Pack output:
          out_pkt.data.range(31,16) = y_r.range(15,0)  ← real
          out_pkt.data.range(15,0)  = y_i.range(15,0)  ← imag
          out_pkt.last = (b == 20)  ← TLAST on last beam of each snapshot
        Write to y_out AXI-Stream
```

After 1000 snapshots, the kernel has produced 21000 output beats on y_out
(21 beams × 1000 snapshots), then asserts ap_done.

The W^H coefficient ROMs (`w_real[21][64]`, `w_imag[21][64]`) are synthesized
as on-chip Block RAM primitives, initialized from `.dat` files generated from
the MATLAB beamforming design. They are declared `static` and initialized only
once via an `initialized` flag — this is a one-time latency of 21×64 = 1344
cycles, amortized across all subsequent calls to the kernel.

### 5.4 Stage 3: Covariance estimation — y_out to x_in handshake

```
rdbs_kernel_0/y_out  ──▶  covariance_estimation_0/x_in
  (AXI4-Stream, TDATA=64-bit, TLAST driven by RDBS on last beam per snapshot)
```

This connection is a direct point-to-point AXI4-Stream link — no FIFO, no
SmartConnect in between. The TVALID/TREADY handshake is directly between
the two IPs. If Covariance is still processing a previous snapshot's data
when RDBS tries to write the next one, TREADY goes low and RDBS stalls
(its BEAM_LOOP pipeline stalls mid-write). This is correct behavior and
fully protocol-compliant.

The covariance kernel sees the same 64-bit TDATA format that RDBS outputs,
and both use the same `axis_data_t` struct definition — so the bit-field
assignments are consistent end-to-end.

### 5.5 Stage 4: Covariance kernel — R matrix computation

```
Internal data flow (from covariance_matrix.cpp):

PHASE 1 — Stream-read into on-chip buffer:
│
│  LOAD_SNAPSHOT (t = 0..999):
│    LOAD_CHANNEL (ch = 0..20):   [pipelined, II=1]
│      Read one beat from y_in AXI-Stream
│      Unpack: X_buf[ch][t].re = pkt.data[31:16]
│              X_buf[ch][t].im = pkt.data[15:0]
│
│  Result: X_buf[21][1000] on-chip — 21000 complex samples of type
│           ap_fixed<16,4>, stored in BRAM partitioned complete on dim=1
│           (so all 21 channel values for any given snapshot t are
│            accessible in a single cycle, enabling II=1 in Phase 2)
│
PHASE 2 — Compute R = (1/K) × Y × Y^H:
│
│  OUTER_I (i = 0..20):
│    OUTER_J (j = i..20):          ← upper triangle only (231 pairs)
│      acc_re = acc_im = 0
│      INNER_K (t = 0..999):       [pipelined, II=1]
│        cmac_conj(acc_re, acc_im,
│                  X_buf[i][t], X_buf[j][t])
│        ← acc += X_buf[i][t] × conj(X_buf[j][t])
│      r_re = acc_re × (1/1000)    ← multiply by inv_K (compile-time constant)
│      r_im = acc_im × (1/1000)
│      R_buf[i][j] = (r_re, r_im)  ← upper triangle
│      R_buf[j][i] = (r_re, -r_im) ← lower triangle (Hermitian symmetry)
│
│  All accumulations use ap_fixed<48,18> to prevent saturation.
│  Maximum accumulator value: |sample|² × K = 64 × 1000 = 64000 → 17 int bits.
│  18 integer bits gives safe headroom. Final result cast to ap_fixed<32,9>.
│
PHASE 3 — Burst-write R to DDR:
│
│  WRITE_ROW (i = 0..20):
│    WRITE_COL (j = 0..20):        [pipelined, II=1]
│      R[i][j] = R_buf[i][j]       ← AXI4 master write via m_axi_gmem
│
│  This generates a 441-entry AXI4 burst write sequence to DDR address
│  DDR_R_BASE = 0x10100000, writing the full 21×21 = 441 complex pairs
│  (441 × 8 bytes = 3528 bytes) as a coherent block.
```

The AXI4 master port `m_axi_gmem` on covariance_estimation_0 connects to
NoC S08_AXI. The NoC routes these write transactions to the DDR MC, which
performs the actual DRAM write operations asynchronously from the kernel's
perspective (the AXI4 write response channel signals completion).

### 5.6 Stage 5: A72 passes DDR addresses to MVDR kernel

After Covariance signals ap_done:

```
A72 executes Xil_DCacheInvalidateRange(DDR_R_BASE, R_SIZE_BYTES)
  ← Forces the A72's cache to discard any stale lines covering this range,
    so subsequent reads fetch fresh data from DDR rather than stale cache.

A72 writes via AXI-Lite (SmartConnect_0 M02 → mvdr_weights s_axi_control):
  R_in  address = 0x10100000   (R matrix in DDR)
  a_in  address = 0x10200000   (steering vector in DDR)
  w_out address = 0x10300000   (output weights destination in DDR)
  ap_start = 1
```

### 5.7 Stage 6: MVDR kernel — weight computation

```
Internal data flow (from mvdr_kernel.cpp):

PHASE 1 — Load R and a into augmented float matrix A[21][22]:
│
│  LOAD_R (i = 0..20):            [AXI4 burst read, m_axi_gmem0]
│    LOAD_COL (j = 0..20):        [pipelined, II=1]
│      A_r[i][j] = (float)R_in[i][j].re
│      A_i[i][j] = (float)R_in[i][j].im
│    A_r[i][21] = (float)a_in[i].re   ← augmented column
│    A_i[i][21] = (float)a_in[i].im
│
│  Note: ap_fixed<32,9> FRAC=23 exactly matches float32's 23-bit mantissa,
│  so this cast is lossless. Internal computation uses float32 throughout
│  because the Cholesky condition number of R can be ~100-1000 (two strong
│  jammers at 20 dB above noise), causing back-substitution values up to
│  ~60000 — far beyond any practical ap_fixed integer range.
│
PHASE 2 — Givens QR decomposition:
│
│  GIVENS_OUTER (p = 0..19):
│    GIVENS_INNER (q = p+1..20):   [sequential — loop-carried dependency]
│      givens_rotate_d(A_r, A_i, p, q, p):
│        r = hls::sqrtf(|A[p][p]|² + |A[q][p]|²)  ← CORDIC sqrt
│        c = conj(A[p][p]) / r
│        s = conj(A[q][p]) / r
│        GIVENS_COL (j = p..21):   [pipelined, II=2]
│          new A[p][j] =  c·A[p][j] + s·A[q][j]
│          new A[q][j] = -s*·A[p][j] + c*·A[q][j]
│
│  After 210 rotations (n*(n-1)/2 = 21*20/2), A[0:20][0:20] is upper
│  triangular U, and A[0:20][21] is Q^H × a (the transformed RHS).
│
PHASE 3 — Complex back-substitution (x = U⁻¹ · c → R⁻¹ · a):
│
│  BACK_SUB_OUTER (i = 20..0):    [sequential]
│    acc = A[i][21]               ← transformed RHS
│    BACK_SUB_INNER (j = i+1..20): [pipelined, II=1]
│      acc -= U[i][j] · x[j]     ← complex multiply-subtract
│    x[i] = acc / U[i][i]        ← complex division via conjugate multiply
│
PHASE 4 — Normalise: w = x / Re(a^H × x):
│
│  denom = Σ_i (a_r[i]·x_r[i] + a_i[i]·x_i[i])   [pipelined, II=1]
│  w[i] = x[i] / denom
│
PHASE 5 — Quantise and write output:
│
│  WRITE_WEIGHTS (i = 0..20):     [pipelined, II=1, m_axi_gmem2 write]
│    w_out[i].re = ap_fixed<32,9>(x_r[i] × inv_denom)
│    w_out[i].im = ap_fixed<32,9>(x_i[i] × inv_denom)
```

The three AXI4 master ports (gmem0/R, gmem1/a, gmem2/w) are merged by
SmartConnect_mem_0 (3 slaves, 1 master) before entering the NoC at S09_AXI,
since the NoC cannot accommodate three separate PL slave ports for a single
kernel without a merger.

```
mvdr_weights_0/m_axi_gmem0 ──┐
mvdr_weights_0/m_axi_gmem1 ──┼──▶ SmartConnect_mem_0 ──▶ NoC S09_AXI ──▶ DDR
mvdr_weights_0/m_axi_gmem2 ──┘
```

The SmartConnect handles arbitration between the three masters transparently —
if gmem0 (reading R) and gmem1 (reading a) issue read requests simultaneously,
SmartConnect arbitrates and serializes them onto the single NoC port.

### 5.8 Stage 7: A72 reads output weights from DDR

```
A72 executes Xil_DCacheInvalidateRange(DDR_W_BASE, W_SIZE_BYTES)
  ← Same reason as before: ensures fresh DDR data, not cached stale zeros

A72 executes memcpy(w, DDR_W_BASE, W_SIZE_BYTES)
  ← Copies 168 bytes (21 × 8 bytes, interleaved int32 re/im pairs) into
     a local int32_t w[42] array

print_q9_23() converts each int32 raw value to a human-readable decimal:
  float_value = raw / 2^23 = raw / 8388608
```

---

## 6. AXI NoC — The Central Interconnect

The AXI NoC is the most critical architectural component in a Versal design.
Unlike a traditional AXI interconnect or SmartConnect, the NoC is a
network fabric with dedicated switching infrastructure in silicon — it is not
synthesized from LUT logic but from hard IP blocks within the Versal device.

### 6.1 Slave Port Assignment and Connectivity

```
NoC Slave Port   Connected To              MC Port Assigned
─────────────────────────────────────────────────────────────
S00_AXI          CIPS FPD_CCI_NOC_0       MC3 (C3_DDR_LOW0/LOW1)
S01_AXI          CIPS FPD_CCI_NOC_1       MC2 (C2_DDR_LOW0/LOW1)
S02_AXI          CIPS FPD_CCI_NOC_2       MC0 (C0_DDR_LOW0/LOW1)
S03_AXI          CIPS FPD_CCI_NOC_3       MC1 (C1_DDR_LOW0/LOW1)
S04_AXI          CIPS LPD_AXI_NOC_0       MC3 (C3_DDR_LOW0/LOW1)
S05_AXI          CIPS PMC_NOC_AXI_0       MC2 (C2_DDR_LOW0/LOW1)
S06_AXI          AXI DMA M_AXI_MM2S       MC0 (C0_DDR_LOW0/LOW1)
S07_AXI          Covariance m_axi_gmem     MC0 (C0_DDR_LOW0/LOW1)
S08_AXI          SmartConnect_mem M00_AXI  MC0 (C0_DDR_LOW0/LOW1)
```

The FPD_CCI ports are distributed across all four MC ports (MC0–MC3) by
Vivado's Connection Automation. This is by design: the four CCI ports
represent four independent cache-coherent streams from the A72's L2 cache,
and spreading them across four DDR memory controller ports enables up to
4× the peak memory bandwidth versus all being on a single port.

All four MC ports feed the same physical DDR4 DIMM — the NoC's memory
controller implements interleaving across the four ports to maximize DRAM
bank utilization.

### 6.2 Clock Association (aclk assignments)

```
NoC aclk port    Clock Source                    Feeds slave ports
──────────────────────────────────────────────────────────────────
aclk0            fpd_cci_noc_axi0_clk (FPD)     S00_AXI
aclk1            fpd_cci_noc_axi1_clk (FPD)     S01_AXI
aclk2            fpd_cci_noc_axi2_clk (FPD)     S02_AXI
aclk3            fpd_cci_noc_axi3_clk (FPD)     S03_AXI
aclk4            lpd_axi_noc_clk (LPD)           S04_AXI
aclk5            pmc_axi_noc_axi0_clk (PMC)      S05_AXI
aclk6            clk_wizard_0/clk_out1 (300 MHz) S06, S07, S08
sys_clk0         ddr4_dimm1_sma_clk (board SMA)  DDR4 MC reference clock
```

---

## 7. Fixed-Point Type Consistency Across the Pipeline

One of the most important design decisions in this system is ensuring that
the fixed-point representations match at every inter-kernel boundary, so that
bit patterns written by one kernel are correctly interpreted by the next.

```
Stage           Type            Format          Range           Frac bits
────────────────────────────────────────────────────────────────────────────
DMA → RDBS      ap_fixed<16,1>  real part        [-1, +1)        15
input samples   packed 32-bit   [31:16]=re       in low 32 bits  of 64-bit
                                [15:0]=im         of TDATA
                                (RDBS confirmed)

RDBS → Cov      ap_fixed<16,4>  beamspace out    [-8, +8)        12
y_out / y_in    packed 32-bit   [31:16]=re       WHY 4 int bits:
                                [15:0]=im         coherent sum of 64 elements,
                                                  each ≤ 1/√64 × 1.0 = 0.125,
                                                  max = 64 × 0.125 = 8

Cov internal    ap_fixed<48,18> accumulator      [-131072,+131072) 30
                                                  |sample|²×K = 64×1000=64000
                                                  → 17 int bits needed, 18 safe

Cov → DDR       ap_fixed<32,9>  R matrix entries [-256, +256)    23
R[21][21]       cresult_t       re and im stored  struct: {result_t re; im;}
                                as adjacent int32

DDR → MVDR      ap_fixed<32,9>  R_in, a_in       [-256, +256)    23
R and a         ccov_t/         cast to float32
                cweight_t       for QR computation

MVDR → DDR      ap_fixed<32,9>  weights w_out    [-256, +256)    23
w[21]           cweight_t       quantized from
                                float32 result
```

The deliberate match between ap_fixed<32,9> (FRAC=23) and float32's 23-bit
mantissa means the fixed-to-float cast inside the MVDR kernel (`(float)R_in[i][j].re`)
is exact — no precision is lost at that conversion. The full numerical
precision of the covariance computation is preserved into the MVDR solver.

---

## 8. Interrupt Architecture

All four peripherals generate interrupts to signal completion to the A72,
avoiding the need for the processor to poll registers in a tight busy-wait
loop (though the current diagnostic version of `main.c` does poll with a
timeout, the interrupt infrastructure is present and tested).

```
rdbs_kernel_0/interrupt          ──▶ CIPS pl_ps_irq0 (LPD IRQ0)
covariance_estimation_0/interrupt ──▶ CIPS pl_ps_irq1 (LPD IRQ1)
mvdr_weights_0/interrupt          ──▶ CIPS pl_ps_irq2 (LPD IRQ2)
axi_dma_0/mm2s_introut            ──▶ CIPS pl_ps_irq3 (LPD IRQ3)
```

Each interrupt line is level-high sensitive (confirmed from rdbs_kernel's
component.xml: `<spirit:name>SENSITIVITY</spirit:name> <spirit:value>LEVEL_HIGH</spirit:value>`).
The CIPS routes these through the GIC-500 interrupt controller to the A72
interrupt vectors. The interrupt is generated when the kernel's ap_done
bit goes high, and is cleared by reading the IP_ISR register in the kernel's
AXI-Lite register block.

---

## 9. Summary — Data Flow in One Sentence Per Stage

```
1. main.c (A72)      memcpy + cache flush → input_samples arrive in DDR
2. A72 → DMA         AXI-Lite start command, source address, byte count
3. DMA → NoC → DDR  AXI4 read bursts fetch 512000 bytes from DDR_INPUT_BASE
4. DMA → RDBS        64000 × 64-bit AXIS beats, one complex sample per beat
5. RDBS (hardware)   21×64 complex MAC per snapshot × 1000 → 21000 AXIS beats out
6. RDBS → Cov        21000 × 64-bit AXIS beats, one beamspace sample per beat
7. Cov (hardware)    21×21 Hermitian outer-product accumulation → R matrix
8. Cov → NoC → DDR  AXI4 burst write, 441 complex pairs (3528 bytes) to DDR
9. A72               Cache invalidate, pass DDR addresses to MVDR via AXI-Lite
10. MVDR → NoC → DDR AXI4 burst reads of R (3528 B) and a (168 B) from DDR
11. MVDR (hardware)  Givens QR + back-substitution + normalisation → 21 weights
12. MVDR → NoC → DDR AXI4 burst write, 21 complex weights (168 bytes) to DDR
13. A72              Cache invalidate + memcpy → weights in local array → printed
```

---

## 10. Timing and Resource Summary

Implementation results on VCK190 (Vivado 2025.2):

```
Metric                          Value           Status
────────────────────────────────────────────────────────────
WNS (Worst Negative Slack)      +3.272 ns       PASS (must be ≥ 0)
WHS (Worst Hold Slack)          +0.029 ns       PASS (must be ≥ 0)
TNS (Total Negative Slack)      0.000 ns        PASS
THS (Total Hold Slack)          0.000 ns        PASS
Failing Endpoints               0               PASS
Clock period (300 MHz target)   3.333 ns        3.272 ns slack = 98% utilised

Methodology warnings:
  TIMING-2: Invalid primary clock source pin — benign, caused by NoC
             internal clock topology not matching Vivado's primary clock
             constraint rules; does not affect silicon timing
  TIMING-4: Clock redefinition on tree — same root cause, same conclusion
  CLKC-72/75: MMCME5 advisory — informational only
```

All user-specified timing constraints are met. The +3.272 ns setup slack
provides significant margin, suggesting the 300 MHz clock could potentially
be pushed to approximately 330–340 MHz without closure risk, though this was
not attempted since 300 MHz already meets the compute throughput requirements.

---

*Document prepared as part of ISRO ISTRAC MVDR Hardware Acceleration Project*
*Platform: Versal VCK190 | Tool version: Vivado/Vitis HLS 2025.2*
