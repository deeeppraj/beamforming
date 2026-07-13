# MVDR Beamformer — Block Design Recreation Guide
## Vivado 2025.2 | Versal VCK190 | Linux

> **This document was written after a complete successful build and implementation
> on Windows and captures every gotcha encountered. Follow the steps in exact
> order and you will reach timing closure without errors.**

---

## Prerequisites

Before opening Vivado, confirm these are in place on your Linux machine:

```
1. Vivado 2025.2 installed and licensed for xcvc1902
2. VCK190 board files installed:
     source /opt/Xilinx/Vivado/2025.2/settings64.sh
     vivado -version   ← should print 2025.2
3. Your 3 HLS IP folders already present, each containing component.xml:
     /path/to/ip/rdbs_kernel/component.xml
     /path/to/ip/covariance_kernel/component.xml
     /path/to/ip/mvdr_kernel/component.xml
4. Board support package (BSP) for VCK190 installed in Vivado:
     Check: Tools > Board Repository > should list Xilinx Versal VCK190 v3.4
     If missing: download from Xilinx Board Store or copy board files to
     ~/.Xilinx/Vivado/2025.2/xhub/board_store/xilinx_board_store/
```

---

## SECTION 1 — Project Creation

### 1.1 Launch Vivado and create project

```
File > New Project
  Project Name: mvdr
  Project Location: /home/<user>/mvdr_project
  Project Type: RTL Project  (NOT HLS Project)
  Next
```

### 1.2 Select board (NOT part)

On the "Default Part" screen:
```
Click the "Boards" tab (not "Parts")
Search: VCK190
Select: Xilinx Versal VCK190 Evaluation Platform  (version 3.4)
Next > Finish
```

> **Critical:** Selecting the board (not just the part) enables Block Automation
> for CIPS which auto-configures DDR4, clocks, and MIO from board preset.
> If you select the part number directly, automation will not offer DDR4 setup.

### 1.3 Add HLS IP repositories

```
Tools > Settings > IP > Repository
Click "+" button
Add each of the 3 IP paths:
  /path/to/ip/rdbs_kernel
  /path/to/ip/covariance_kernel
  /path/to/ip/mvdr_kernel
Click OK
```

You should see Vivado report:
```
INFO: Loaded user IP repository '/path/to/ip/rdbs_kernel'     ← 1 IP found
INFO: Loaded user IP repository '/path/to/ip/covariance_kernel' ← 1 IP found
INFO: Loaded user IP repository '/path/to/ip/mvdr_kernel'       ← 1 IP found
```

If it says "0 IPs found" for any repo, the component.xml is missing — your
HLS IP export did not complete. Re-export from Vitis HLS:
```
Solution > Export RTL > Format: Vivado IP
```

### 1.4 Create Block Design

```
Flow Navigator (left panel) > IP Integrator > Create Block Design
Name: mvdr_system
OK
```

An empty canvas opens. All remaining steps happen on this canvas.

---

## SECTION 2 — Add and Configure IPs (in this exact order)

### STEP 1: Add CIPS and run Block Automation

**Add the IP:**
```
Ctrl+I (Add IP) > search "versal cips" > double-click to add
```

**Run Block Automation immediately:**
```
A green banner appears at top of canvas: "Designer Assistance available: Run Block Automation"
Click it
OR: right-click versal_cips_0 > Run Block Automation
```

**In the Block Automation dialog, set exactly:**

| Field | Value |
|-------|-------|
| Design Flow | Full System |
| Apply Board Preset | **Yes** |
| Debug Configuration | JTAG |
| PL Clocks | **1** |
| PL Resets | **1** |
| Memory Controller Type | **DDR4** |
| Configure NoC | **Add new AXI NoC** |

Click **OK**.

**What automation creates for you (do not create these manually):**
- `axi_noc_0` — AXI NoC with DDR4 memory controller
- Connects CIPS FPD_CCI_NOC_0/1/2/3 → NoC S00/S01/S02/S03
- Connects CIPS LPD_AXI_NOC_0 → NoC S04
- Connects CIPS PMC_NOC_AXI_0 → NoC S05
- Wires all NoC clock ports (aclk0–aclk5) from CIPS clock outputs
- Creates external port `ddr4_dimm1` (DDR4 DIMM interface to board)
- Creates external port `ddr4_dimm1_sma_clk` (DDR4 reference clock from board SMA)
- Exposes `pl0_ref_clk` and `pl0_resetn` on CIPS

Canvas should now show CIPS and NoC connected with 6 buses and DDR4 going off-canvas.

---

### STEP 2: Enable M_AXI_FPD on CIPS

The PS needs a master AXI port to reach HLS kernel control registers.
Block Automation does NOT enable this by default.

```
Double-click versal_cips_0
Go to: PS PMC tab
Navigate to: AXI Options (or search "FPD" in the search bar at top of the dialog)
```

Set:

| Parameter | Value |
|-----------|-------|
| M_AXI_FPD | **Enable** (check the box) |
| M_AXI_FPD Data Width | **128** |
| M_AXI_LPD | **Disable** (leave unchecked — we don't need it) |

Click **OK**.

After this, `M_AXI_FPD` and `m_axi_fpd_aclk` ports appear on the CIPS block on canvas.

---

### STEP 3: Enable PL-PS interrupts on CIPS

The HLS kernels and DMA send interrupt signals to the A72. These must be
enabled inside CIPS before the ports appear on the canvas.

```
Double-click versal_cips_0
PS PMC tab > search "interrupt" or scroll to "PL to PS Interrupts" section
```

Under **PL to PS Interrupts > LPD** section, enable:

| IRQ | Enable | Maps to |
|-----|--------|---------|
| IRQ 0 | ✅ | rdbs_kernel interrupt |
| IRQ 1 | ✅ | covariance_estimation interrupt |
| IRQ 2 | ✅ | mvdr_weights interrupt |
| IRQ 3 | ✅ | axi_dma mm2s_introut |

Leave all FPD IRQs (IRQ 8–15) unchecked.

Click **OK**.

After this, `pl_ps_irq0`, `pl_ps_irq1`, `pl_ps_irq2`, `pl_ps_irq3` appear
on the CIPS block on canvas.

---

### STEP 4: Reconfigure AXI NoC

The NoC was created by automation with 6 slave ports (S00–S05). We need 9
total (adding DMA, covariance gmem, and mvdr SmartConnect).

```
Double-click axi_noc_0
```

**General Tab — change these values:**

| Parameter | Value | Why |
|-----------|-------|-----|
| Number of AXI Slave Interfaces | **9** | S00–S08 (see table below) |
| Number of AXI Master Interfaces | 0 | NoC connects to DDR via MC, not AXI master |
| Number of AXI Clocks | **7** | aclk0–aclk5 from CIPS + aclk6 for PL (300 MHz) |
| Memory Controller | Single Memory Controller | One DDR4 DIMM |
| Number of Memory Controller Ports | 4 | 4 interleaved ports for bandwidth |
| DDR Address Region 0 | DDR LOW0 | 0x0000_0000 to 0x7FFF_FFFF (2 GB) |

> **Why 9 slaves?**
> S00–S03: FPD_CCI (4 cache-coherent PS ports, all mandatory)
> S04: LPD
> S05: PMC
> S06: DMA M_AXI_MM2S (reads input from DDR, streams to RDBS)
> S07: covariance_estimation m_axi_gmem (writes R matrix to DDR)
> S08: mvdr SmartConnect output (reads R,a from DDR; writes weights to DDR)

**Connectivity Tab:**

Click **Connect All** button at the top of the Connectivity tab.

This distributes all 9 slaves across the 4 MC ports for balanced bandwidth.
The exact distribution (which slave goes to which MC port) is determined by
Vivado's algorithm — it does not matter which specific MC port each slave uses,
as all 4 ports access the same physical DDR4 DIMM. Do not manually change the
checkboxes after clicking Connect All.

> **Known issue:** After clicking Connect All you may see slaves distributed
> unevenly across MC Port 0, 1, 2, 3 — this is correct and intentional.
> Do NOT try to force all slaves to MC Port 0. It works either way, but
> the interleaved distribution gives better memory bandwidth.

Click **OK**.

---

### STEP 5: Add Clocking Wizard

```
Ctrl+I > search "Clocking Wizard" > double-click to add
```

**Double-click clk_wizard_0 to configure:**

| Parameter | Value |
|-----------|-------|
| Component Name | clk_wizard_0 |
| Clocking Options > Primary Input Clock > Input Frequency | 100.000 MHz |
| Clocking Options > Primitive | MMCME5 |
| Clocking Options > Input Clock Source | **No buffer** |
| Output Clocks > CLK_OUT1 > Requested Output Freq | **300.000 MHz** |
| Output Clocks > CLK_OUT2 through CLK_OUT7 | All **disabled** (uncheck Used) |
| Output Clocks > Reset Type | **Active Low** |

> **Why Active Low?** pl0_resetn from CIPS is active-low. Selecting active-low
> here means we can connect pl0_resetn directly to the Clocking Wizard reset
> pin without needing an inverter.

> **Why No buffer?** pl0_ref_clk from CIPS is already a buffered clock coming
> through Versal's clock management. Selecting No buffer prevents double-buffering
> which would cause timing issues and a TIMING-2 methodology warning (though
> even with this warning, timing closure was achieved — the warning is benign
> but cleaner to avoid).

Click **OK**.

---

### STEP 6: Add Processor System Reset

```
Ctrl+I > search "Processor System Reset" > double-click to add
```

No configuration needed in the dialog. Click OK with defaults.

---

### STEP 7: Add AXI DMA

```
Ctrl+I > search "AXI Direct Memory Access" > double-click to add
```

**Double-click axi_dma_0 to configure:**

| Parameter | Value | Notes |
|-----------|-------|-------|
| Enable Scatter Gather Engine | ❌ Unchecked | Not needed, simpler without |
| Enable Micro DMA | ❌ Unchecked | |
| Width of Buffer Length Register | **23** | 2^23 = 8 MB max transfer; our input is 512 KB |
| Address Width | **64** | Required for Versal 64-bit address space |
| Enable Read Channel (MM2S) | ✅ Checked | DMA reads from DDR, streams to RDBS |
| MM2S Memory Map Data Width | **64** | Must match rdbs_kernel x_in TDATA width |
| MM2S Stream Data Width | **64** | Confirmed via component.xml: TDATA_NUM_BYTES=8 |
| MM2S Max Burst Size | **256** | Maximum DDR burst efficiency |
| Enable Write Channel (S2MM) | ❌ **Unchecked** | Not needed — MVDR writes weights via gmem |

> **Critical — why 64-bit stream width?**
> rdbs_kernel synthesizes its `axis_data_t {ap_int<32> data; bool last}` struct
> into a 64-bit TDATA bus (confirmed from component.xml: TDATA_NUM_BYTES=8,
> HAS_TLAST=0). The `bool last` field gets packed into TDATA[32], not onto
> the protocol TLAST wire. The DMA stream must be 64-bit to match.
> Setting this to 32-bit causes RDBS to receive half the expected number of
> beats and stall forever.

> **Critical — why Buffer Length Register = 23?**
> Default is 14 bits = max 16 KB per transfer. Our input is 512 KB (64000 × 8).
> With 14 bits you'd need to chunk the transfer in software, which breaks the
> continuous AXIS stream RDBS expects (stalls between chunks confuse the kernel).
> 23 bits = 8 MB max, covers 512 KB in a single uninterrupted transfer.

Click **OK**.

---

### STEP 8: Add AXI SmartConnect (control path)

```
Ctrl+I > search "AXI SmartConnect" > double-click to add
```

**Double-click smartconnect_0 to configure:**

| Parameter | Value |
|-----------|-------|
| Number of Slave Interfaces | **1** |
| Number of Master Interfaces | **4** |

Click OK.

This SmartConnect fans out the PS's single M_AXI_FPD port to reach all
4 control register slaves: rdbs_kernel, covariance_estimation, mvdr_weights,
and axi_dma.

---

### STEP 9: Add AXI SmartConnect (memory path)

```
Ctrl+I > search "AXI SmartConnect" > double-click to add
```

This creates `smartconnect_1` by default. Rename it to `smartconnect_mem_0`
for clarity (right-click > Rename, or just remember it by its instance name).

**Double-click to configure:**

| Parameter | Value |
|-----------|-------|
| Number of Slave Interfaces | **3** |
| Number of Master Interfaces | **1** |

Click OK.

> **Why is this needed?** mvdr_weights has 3 separate AXI4 master ports
> (gmem0 for R, gmem1 for a, gmem2 for weights). The NoC cannot accept 3
> separate PL master ports for a single kernel unless we add 3 more slave
> ports to the NoC. Instead, we merge all 3 into one using this SmartConnect,
> saving NoC ports and simplifying the design.

---

### STEP 10: Add HLS Kernels

Add all three, one at a time:

```
Ctrl+I > search "rdbs" > add rdbs_kernel
Ctrl+I > search "covariance" > add Covariance_estimation
Ctrl+I > search "mvdr" > add Mvdr_weights
```

No configuration needed for any of them — all settings are fixed by the HLS
synthesis. They appear on canvas with their ports.

**Verify their VLNVs in IP Catalog match exactly:**
```
xilinx.com:hls:rdbs_kernel:1.0
xilinx.com:hls:covariance_estimation:1.0
xilinx.com:hls:mvdr_weights:1.0
```

If any kernel shows as a black box (greyed out), the IP repository path is
wrong. Re-check Settings > IP > Repository.

---

## SECTION 3 — Wire All Connections

Make the following connections by hovering over a port until the pencil cursor
appears, then clicking and dragging to the destination port. Alternatively,
use the Tcl console to make connections programmatically (see Section 6).

### 3.1 Clock Connections

| Source Port | Destination Port |
|-------------|-----------------|
| `versal_cips_0/pl0_ref_clk` | `clk_wizard_0/clk_in1` |
| `clk_wizard_0/clk_out1` | `proc_sys_reset_0/slowest_sync_clk` |
| `clk_wizard_0/clk_out1` | `axi_dma_0/m_axi_mm2s_aclk` |
| `clk_wizard_0/clk_out1` | `axi_dma_0/s_axi_lite_aclk` |
| `clk_wizard_0/clk_out1` | `rdbs_kernel_0/ap_clk` |
| `clk_wizard_0/clk_out1` | `covariance_estimation_0/ap_clk` |
| `clk_wizard_0/clk_out1` | `mvdr_weights_0/ap_clk` |
| `clk_wizard_0/clk_out1` | `smartconnect_0/aclk` |
| `clk_wizard_0/clk_out1` | `smartconnect_mem_0/aclk` |
| `clk_wizard_0/clk_out1` | `axi_noc_0/aclk6` |
| `versal_cips_0/m_axi_fpd_aclk` | `smartconnect_0/aclk` |

> **Note on smartconnect_0/aclk:** This port will already be connected to
> clk_wizard_0/clk_out1 from the table above. However, per the actual working
> design, the FPD clock (m_axi_fpd_aclk) is the correct clock for the
> control-path SmartConnect since it runs in the FPD clock domain. In
> Vivado 2025.2, SmartConnect handles the crossing internally — connect
> m_axi_fpd_aclk to smartconnect_0/aclk and clk_out1 to all others.

**Corrected control SmartConnect clock:**

| Source Port | Destination Port |
|-------------|-----------------|
| `versal_cips_0/m_axi_fpd_aclk` | `smartconnect_0/aclk` |

> The FPD clock and kernel clock (300 MHz) may differ. SmartConnect contains
> clock-domain crossing FIFOs internally. Connecting m_axi_fpd_aclk to
> smartconnect_0/aclk is the architecturally correct choice.

---

### 3.2 Reset Connections

| Source Port | Destination Port |
|-------------|-----------------|
| `versal_cips_0/pl0_resetn` | `proc_sys_reset_0/ext_reset_in` |
| `clk_wizard_0/locked` | `proc_sys_reset_0/dcm_locked` |
| `proc_sys_reset_0/peripheral_aresetn` | `rdbs_kernel_0/ap_rst_n` |
| `proc_sys_reset_0/peripheral_aresetn` | `covariance_estimation_0/ap_rst_n` |
| `proc_sys_reset_0/peripheral_aresetn` | `mvdr_weights_0/ap_rst_n` |
| `proc_sys_reset_0/peripheral_aresetn` | `axi_dma_0/axi_resetn` |
| `proc_sys_reset_0/peripheral_aresetn` | `smartconnect_0/aresetn` |
| `proc_sys_reset_0/peripheral_aresetn` | `smartconnect_mem_0/aresetn` |

---

### 3.3 CIPS to NoC (already auto-connected — verify only)

These were created by Block Automation. Verify they exist:

| Source | Destination |
|--------|-------------|
| `versal_cips_0/FPD_CCI_NOC_0` | `axi_noc_0/S00_AXI` |
| `versal_cips_0/FPD_CCI_NOC_1` | `axi_noc_0/S01_AXI` |
| `versal_cips_0/FPD_CCI_NOC_2` | `axi_noc_0/S02_AXI` |
| `versal_cips_0/FPD_CCI_NOC_3` | `axi_noc_0/S03_AXI` |
| `versal_cips_0/LPD_AXI_NOC_0` | `axi_noc_0/S04_AXI` |
| `versal_cips_0/PMC_NOC_AXI_0` | `axi_noc_0/S05_AXI` |

---

### 3.4 PS Control Path

| Source | Destination |
|--------|-------------|
| `versal_cips_0/M_AXI_FPD` | `smartconnect_0/S00_AXI` |
| `smartconnect_0/M00_AXI` | `rdbs_kernel_0/s_axi_control` |
| `smartconnect_0/M01_AXI` | `covariance_estimation_0/s_axi_control` |
| `smartconnect_0/M02_AXI` | `mvdr_weights_0/s_axi_control` |
| `smartconnect_0/M03_AXI` | `axi_dma_0/S_AXI_LITE` |

---

### 3.5 DMA to NoC

| Source | Destination |
|--------|-------------|
| `axi_dma_0/M_AXI_MM2S` | `axi_noc_0/S06_AXI` |

> DMA write channel (S2MM) is disabled — no S07 connection needed.
> S07 can be left unconnected or the NoC can be reduced to 8 slaves
> if you want a cleaner design. Either way is fine.

---

### 3.6 HLS gmem to NoC

| Source | Destination |
|--------|-------------|
| `covariance_estimation_0/m_axi_gmem` | `axi_noc_0/S07_AXI` |
| `mvdr_weights_0/m_axi_gmem0` | `smartconnect_mem_0/S00_AXI` |
| `mvdr_weights_0/m_axi_gmem1` | `smartconnect_mem_0/S01_AXI` |
| `mvdr_weights_0/m_axi_gmem2` | `smartconnect_mem_0/S02_AXI` |
| `smartconnect_mem_0/M00_AXI` | `axi_noc_0/S08_AXI` |

---

### 3.7 AXI-Stream Pipeline

| Source | Destination | Notes |
|--------|-------------|-------|
| `axi_dma_0/M_AXIS_MM2S` | `rdbs_kernel_0/x_in` | 64-bit TDATA stream |
| `rdbs_kernel_0/y_out` | `covariance_estimation_0/x_in` | 64-bit TDATA stream |

> mvdr_weights_0 has no AXI-Stream input. It reads its data entirely via
> AXI4 master ports (m_axi_gmem0/1/2) directly from DDR.

---

### 3.8 Interrupt Connections

| Source | Destination |
|--------|-------------|
| `rdbs_kernel_0/interrupt` | `versal_cips_0/pl_ps_irq0` |
| `covariance_estimation_0/interrupt` | `versal_cips_0/pl_ps_irq1` |
| `mvdr_weights_0/interrupt` | `versal_cips_0/pl_ps_irq2` |
| `axi_dma_0/mm2s_introut` | `versal_cips_0/pl_ps_irq3` |

> If pl_ps_irq0..3 ports are not visible on the CIPS block, go back to
> Step 3 (Section 2) and enable IRQ 0–3 under PL to PS Interrupts > LPD.

---

## SECTION 4 — NoC Clock Association

After wiring aclk6 from clk_wizard_0/clk_out1, you need to tell the NoC
which aclk drives each PL slave port. This is done via the NoC configuration
dialog, not by wiring alone.

```
Double-click axi_noc_0
Go to Inputs tab
```

In the Inputs tab, you will see a table of slave ports with a "Connected To"
column. Verify:

| Slave Port | Connected To |
|------------|--------------|
| S00_AXI | PS Cache Coherent |
| S01_AXI | PS Cache Coherent |
| S02_AXI | PS Cache Coherent |
| S03_AXI | PS Cache Coherent |
| S04_AXI | PS LPD |
| S05_AXI | PS PMC |
| S06_AXI | **PL** |
| S07_AXI | **PL** |
| S08_AXI | **PL** |

S06, S07, S08 should all show "PL" in their "Connected To" column. This is
how the NoC knows to use aclk6 (the 300 MHz PL clock) for those ports.

**Verify aclk6 association in Tcl console:**
```tcl
get_property CONFIG.ASSOCIATED_BUSIF [get_bd_pins axi_noc_0/aclk6]
```
Expected output: `S08_AXI:S07_AXI:S06_AXI`

If the output is empty or missing some ports, the aclk6 association is
incomplete. Fix it:
```tcl
set_property CONFIG.ASSOCIATED_BUSIF {S06_AXI:S07_AXI:S08_AXI} \
    [get_bd_pins axi_noc_0/aclk6]
```

---

## SECTION 5 — Address Editor

```
Window > Address Editor
```

Click the **Auto Assign Address** button (wand/magic wand icon at top-left
of the Address Editor panel).

Vivado will assign all addresses automatically. After assignment, verify:

### 5.1 DDR4 assignments (all masters → DDR)

| Master Space | Expected Segment | Expected Base |
|--------------|-----------------|---------------|
| `axi_dma_0/Data_MM2S` | C0_DDR_LOW0 | 0x0 |
| `covariance_estimation_0/Data_m_axi_gmem` | C0_DDR_LOW0 | 0x0 |
| `mvdr_weights_0/Data_m_axi_gmem0` | C0_DDR_LOW0 | 0x0 |
| `mvdr_weights_0/Data_m_axi_gmem1` | C0_DDR_LOW0 | 0x0 |
| `mvdr_weights_0/Data_m_axi_gmem2` | C0_DDR_LOW0 | 0x0 |
| `versal_cips_0/FPD_CCI_NOC_0..3` | C_DDR_LOW0 | 0x0 |

### 5.2 Control register assignments

| Master Space | Slave Segment | Base Address |
|--------------|--------------|--------------|
| `versal_cips_0/M_AXI_FPD` | rdbs_kernel Reg | 0xA400_0000 |
| `versal_cips_0/M_AXI_FPD` | covariance Reg | 0xA401_0000 |
| `versal_cips_0/M_AXI_FPD` | mvdr_weights Reg | 0xA402_0000 |
| `versal_cips_0/M_AXI_FPD` | axi_dma Reg | 0xA403_0000 |

> **Known issue — covariance gmem address shows as "Excluded":**
> After Auto Assign, `covariance_estimation_0/Data_m_axi_gmem` may show
> its DDR segments as "Excluded" (greyed out with an X icon) rather than
> assigned. This is a Vivado 2025.2 quirk with the NoC addressing path.
>
> **Fix:** In the Address Editor, right-click on the excluded entry under
> covariance_estimation_0/Data_m_axi_gmem and select **Include Segment**.
> Do this for both C0_DDR_LOW0 and C0_DDR_LOW1 entries.
>
> After including them, re-run validate_bd_design. The error about
> "incomplete addressing path" will resolve.

---

## SECTION 6 — Validate Design

```
Tools > Validate Design   (or press F6)
```

### Expected warnings (safe to ignore):

| Warning ID | Message | Why it's safe |
|------------|---------|---------------|
| BD 41-2670 | Incomplete address path | Fixed by including covariance segments above |
| BD 41-3281 | mvdr_weights between SmartConnects | Informational — configure widths if needed |
| TIMING-2 | Invalid primary clock source pin | Benign — NoC internal topology quirk |
| TIMING-4 | Clock redefinition on tree | Benign — same root cause as TIMING-2 |
| CLKC-72 | Substitute PLL for MMCME5 | Advisory only |
| CLKC-75 | MMCME5 with no LOC | Advisory only |

### Errors that must be fixed before proceeding:

| Error | Fix |
|-------|-----|
| `NoC interface S0x_AXI is not driven` | A NoC slave port has no connection — check Section 3.5 |
| `Clock pins not connected to valid clock source` | A kernel's ap_clk or aclk not wired — check Section 3.1 |
| `TDATA_NUM_BYTES mismatch between X and Y` | Stream width mismatch — verify DMA MM2S width is 64 |
| `S_AXIS_S2MM interface is unconnected` | DMA write channel still enabled — disable S2MM in DMA config |
| `bd_rule:versal_cips cannot be found` | Do NOT use Run Block Automation from TCL — use GUI only |

After all errors are resolved and only the expected warnings remain:
```
Ctrl+S (Save)
```

---

## SECTION 7 — Pre-Implementation Verification (Tcl Checks)

Run these in the Vivado Tcl console to confirm key configuration before
committing to a full implementation run:

```tcl
# 1. Confirm DMA stream width matches RDBS (both must be 64-bit)
get_property CONFIG.c_m_axis_mm2s_tdata_width [get_bd_cells axi_dma_0]
# Expected: 64

get_property CONFIG.TDATA_NUM_BYTES [get_bd_intf_pins rdbs_kernel_0/x_in]
# Expected: 8  (8 bytes = 64 bits)

# 2. Confirm clocks match on the AXIS link
get_property CONFIG.FREQ_HZ [get_bd_intf_pins axi_dma_0/M_AXIS_MM2S]
get_property CONFIG.FREQ_HZ [get_bd_intf_pins rdbs_kernel_0/x_in]
# Expected: both 99999900 (or same value)

# 3. Confirm buffer length register is large enough
get_property CONFIG.c_sg_length_width [get_bd_cells axi_dma_0]
# Expected: 23

# 4. Confirm NoC PL clock association
get_property CONFIG.ASSOCIATED_BUSIF [get_bd_pins axi_noc_0/aclk6]
# Expected: S06_AXI:S07_AXI:S08_AXI (order may vary)

# 5. Confirm DMA write channel is disabled
get_property CONFIG.c_include_s2mm [get_bd_cells axi_dma_0]
# Expected: 0

# 6. Confirm RDBS has no TLAST (so last-bit handling in data packing is correct)
get_property CONFIG.HAS_TLAST [get_bd_intf_pins rdbs_kernel_0/x_in]
# Expected: 0
```

If all 6 checks return the expected values, the design configuration is
identical to the working implementation.

---

## SECTION 8 — Generate, Implement, Export

Run these steps in order. Each must complete without errors before proceeding.

### Step 1: Create HDL Wrapper
```
Sources panel > right-click mvdr_system (under Design Sources)
> Create HDL Wrapper
> Select: "Let Vivado manage wrapper and auto-update"
> OK
```
A file `mvdr_system_wrapper.v` appears in Sources.

### Step 2: Generate Block Design
```
Tools > Generate Block Design
Synthesis Options: Out of Context per IP
Click: Generate
```
This pre-synthesizes each IP independently. Takes 10–30 minutes depending
on machine. Do not skip this step — it catches IP-level errors early.

### Step 3: Run Implementation
```
Flow Navigator > Run Implementation
```
When synthesis completes, click OK to proceed to implementation.
Implementation completes in approximately 45 min – 2 hours.

### Step 4: Check Timing Report
```
Reports > Timing Summary (or open impl_1/mvdr_system_wrapper_timing_summary_routed.rpt)
```

**Required values for a valid implementation:**

| Metric | Required | Achieved (reference) |
|--------|----------|---------------------|
| WNS | ≥ 0 ns | +3.272 ns |
| WHS | ≥ 0 ns | +0.029 ns |
| TNS | 0 ns | 0.000 ns |
| THS | 0 ns | 0.000 ns |
| Failing Endpoints | 0 | 0 |

If WNS is negative, do not proceed to Generate Device Image — the design
will not function correctly in silicon even if it programs. Post a message
with the failing paths and we can add timing constraints.

### Step 5: Generate Device Image
```
Flow Navigator > Generate Device Image
```
On Versal, this produces a `.pdi` file instead of a `.bit` file. It takes
an additional 10–20 minutes after implementation.

### Step 6: Export Hardware
```
File > Export > Export Hardware
Output: XSA file
Include device image: Yes
Location: /path/to/your/choice/mvdr_system_wrapper.xsa
Click OK
```

This `.xsa` file is what Vitis needs to create the platform project and
generate BSP drivers for the HLS kernels and DMA.

---

## SECTION 9 — Common Errors and Exact Fixes

| # | Error / Symptom | Root Cause | Fix |
|---|-----------------|------------|-----|
| 1 | Block Automation dialog doesn't offer DDR4 | Board not selected — part was selected instead | Start over: select board, not part, in project creation |
| 2 | `ERROR: bd_rule:versal_cips cannot be found` | TCL script using old automation rule removed in 2025.2 | Use GUI for Block Automation only — do not source automation TCL |
| 3 | HLS IPs show as black box | IP repo paths wrong or component.xml missing | Verify paths in Settings > IP > Repository; re-export HLS IPs if needed |
| 4 | `NoC S07_AXI is not driven` | DMA S2MM was enabled but not connected | Disable Write Channel in DMA config OR connect M_AXI_S2MM to a NoC port |
| 5 | `TDATA_NUM_BYTES mismatch: x_in(8) M_AXIS_MM2S(4)` | DMA stream width set to 32-bit | Set MM2S Stream Data Width to 64 in DMA config |
| 6 | `RDBS: done=0 idle=0 ready=0` on board | Input buffer sent 32-bit words (256 KB) instead of 64-bit words (512 KB) | Regenerate input_samples.h as uint64_t; set INPUT_SIZE_BYTES = N_CHANNELS * N_SNAPSHOTS * 8 |
| 7 | `RDBS: done=0` still after fix #6 | DMA Buffer Length Register too small for 512 KB | Set Width of Buffer Length Register to 23 in DMA config (already done if following this guide) |
| 8 | Covariance gmem shows "Excluded" in Address Editor | Vivado 2025.2 NoC addressing quirk | Right-click excluded segments > Include Segment for both LOW0 and LOW1 |
| 9 | `cannot find a top module` critical warning | HDL wrapper not created | Right-click BD > Create HDL Wrapper |
| 10 | WNS negative after implementation | Clock too tight or missing constraint | Check if 300 MHz is achievable; post failing paths for analysis |
| 11 | `mem write error at 0x0` on board | PS writes to DDR before DDR calibration completes | Ensure you're loading a .pdi (not just .bit); use Vitis platform with XSA |
| 12 | Weights all zero after successful run | Cache not invalidated before reading | Add `Xil_DCacheInvalidateRange(DDR_W_BASE, W_SIZE_BYTES)` before memcpy |
| 13 | M_AXI_LPD connected but nothing uses it | Accidentally enabled M_AXI_LPD in CIPS | Double-click CIPS > disable M_AXI_LPD; removes the "Incomplete Paths" warning |

---

## SECTION 10 — Quick Reference: Full Connection List

One-page summary of every connection in the design.

```
CLOCK CONNECTIONS
─────────────────────────────────────────────────────────────────
versal_cips_0/pl0_ref_clk          →  clk_wizard_0/clk_in1
clk_wizard_0/clk_out1              →  proc_sys_reset_0/slowest_sync_clk
clk_wizard_0/clk_out1              →  axi_dma_0/m_axi_mm2s_aclk
clk_wizard_0/clk_out1              →  axi_dma_0/s_axi_lite_aclk
clk_wizard_0/clk_out1              →  rdbs_kernel_0/ap_clk
clk_wizard_0/clk_out1              →  covariance_estimation_0/ap_clk
clk_wizard_0/clk_out1              →  mvdr_weights_0/ap_clk
clk_wizard_0/clk_out1              →  smartconnect_mem_0/aclk
clk_wizard_0/clk_out1              →  axi_noc_0/aclk6
versal_cips_0/m_axi_fpd_aclk       →  smartconnect_0/aclk
[aclk0–aclk5 auto-connected by Block Automation]

RESET CONNECTIONS
─────────────────────────────────────────────────────────────────
versal_cips_0/pl0_resetn            →  proc_sys_reset_0/ext_reset_in
clk_wizard_0/locked                 →  proc_sys_reset_0/dcm_locked
proc_sys_reset_0/peripheral_aresetn →  rdbs_kernel_0/ap_rst_n
proc_sys_reset_0/peripheral_aresetn →  covariance_estimation_0/ap_rst_n
proc_sys_reset_0/peripheral_aresetn →  mvdr_weights_0/ap_rst_n
proc_sys_reset_0/peripheral_aresetn →  axi_dma_0/axi_resetn
proc_sys_reset_0/peripheral_aresetn →  smartconnect_0/aresetn
proc_sys_reset_0/peripheral_aresetn →  smartconnect_mem_0/aresetn

AXI4 INTERFACE CONNECTIONS
─────────────────────────────────────────────────────────────────
[Auto by Block Automation]
versal_cips_0/FPD_CCI_NOC_0        →  axi_noc_0/S00_AXI
versal_cips_0/FPD_CCI_NOC_1        →  axi_noc_0/S01_AXI
versal_cips_0/FPD_CCI_NOC_2        →  axi_noc_0/S02_AXI
versal_cips_0/FPD_CCI_NOC_3        →  axi_noc_0/S03_AXI
versal_cips_0/LPD_AXI_NOC_0        →  axi_noc_0/S04_AXI
versal_cips_0/PMC_NOC_AXI_0        →  axi_noc_0/S05_AXI

[PS control path — manual]
versal_cips_0/M_AXI_FPD            →  smartconnect_0/S00_AXI
smartconnect_0/M00_AXI             →  rdbs_kernel_0/s_axi_control
smartconnect_0/M01_AXI             →  covariance_estimation_0/s_axi_control
smartconnect_0/M02_AXI             →  mvdr_weights_0/s_axi_control
smartconnect_0/M03_AXI             →  axi_dma_0/S_AXI_LITE

[DMA to DDR — manual]
axi_dma_0/M_AXI_MM2S               →  axi_noc_0/S06_AXI

[HLS gmem to DDR — manual]
covariance_estimation_0/m_axi_gmem  →  axi_noc_0/S07_AXI
mvdr_weights_0/m_axi_gmem0          →  smartconnect_mem_0/S00_AXI
mvdr_weights_0/m_axi_gmem1          →  smartconnect_mem_0/S01_AXI
mvdr_weights_0/m_axi_gmem2          →  smartconnect_mem_0/S02_AXI
smartconnect_mem_0/M00_AXI          →  axi_noc_0/S08_AXI

AXI4-STREAM CONNECTIONS
─────────────────────────────────────────────────────────────────
axi_dma_0/M_AXIS_MM2S              →  rdbs_kernel_0/x_in
rdbs_kernel_0/y_out                 →  covariance_estimation_0/x_in

INTERRUPT CONNECTIONS
─────────────────────────────────────────────────────────────────
rdbs_kernel_0/interrupt             →  versal_cips_0/pl_ps_irq0
covariance_estimation_0/interrupt   →  versal_cips_0/pl_ps_irq1
mvdr_weights_0/interrupt            →  versal_cips_0/pl_ps_irq2
axi_dma_0/mm2s_introut              →  versal_cips_0/pl_ps_irq3
```

---

## SECTION 11 — Address Map (for main.c cross-reference)

| Symbol in main.c | Address | Contents |
|-----------------|---------|----------|
| `XPAR_RDBS_KERNEL_0_BASEADDR` | 0xA400_0000 | rdbs_kernel AXI-Lite regs |
| `XPAR_COVARIANCE_ESTIMATION_0_BASEADDR` | 0xA401_0000 | covariance AXI-Lite regs |
| `XPAR_MVDR_WEIGHTS_0_BASEADDR` | 0xA402_0000 | mvdr_weights AXI-Lite regs |
| `XPAR_AXI_DMA_0_BASEADDR` | 0xA403_0000 | AXI DMA AXI-Lite regs |
| `DDR_INPUT_BASE` | 0x1000_0000 | Input samples (512 KB, uint64_t) |
| `DDR_R_BASE` | 0x1010_0000 | R matrix (3528 bytes) |
| `DDR_A_BASE` | 0x1020_0000 | Steering vector (168 bytes) |
| `DDR_W_BASE` | 0x1030_0000 | Output weights (168 bytes) |

> These addresses are set by main.c, not by Vivado. The hardware doesn't
> know or care what specific DDR address you use as long as it falls within
> the 2 GB DDR4 space (0x0000_0000 to 0x7FFF_FFFF). The AXI-Lite register
> addresses (0xA400_0000 etc.) ARE fixed by the Address Editor — verify them
> in xparameters.h after building the Vitis platform.

---

*MVDR Beamformer — ISRO ISTRAC Hardware Acceleration Project*
*Versal VCK190 | Vivado 2025.2 | Produced after successful implementation*
*WNS = +3.272 ns | WHS = +0.029 ns | Failing endpoints = 0*
