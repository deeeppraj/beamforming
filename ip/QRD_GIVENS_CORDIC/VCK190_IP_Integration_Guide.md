# VCK190 Vivado IP Integration — Complete Step-by-Step Guide
## RDBS + Covariance Estimation + MVDR Weights

---

## Overview of the Three IPs

| IP | Function | AXI-Stream | AXI4 Master | AXI-Lite |
|---|---|---|---|---|
| `rdbs_kernel` | 64-element → 21-beam compressor | x_in (slave), y_out (master) | none | ctrl (num_snapshots, return) |
| `covariance_estimation` | 21×1000 → 21×21 matrix | y_in (slave) | gmem (R output to DDR) | ctrl (R pointer, return) |
| `mvdr_weights` | R⁻¹·a → MVDR weights | none | gmem0 (R_in), gmem1 (a_in), gmem2 (w_out) | ctrl (R_in, a_in, w_out pointers, return) |

---

## What You Will Build

```
[ADC/DMA input]
      |
      | AXI-Stream (64 elements/snapshot)
      v
[rdbs_kernel]
      |
      | AXI-Stream (21 beams/snapshot)
      v
[covariance_estimation] ---AXI4 master---> [AXI NoC] ---> [LPDDR4 DDR]
                                                ^
[mvdr_weights] ----------AXI4 master----------/
      ^
      | AXI-Lite (addresses of R, a, w buffers in DDR)
      |
[CIPS PS] ---AXI-Lite---> [AXI SmartConnect] ---> [rdbs ctrl]
                                               ---> [covariance ctrl]
                                               ---> [mvdr ctrl]
```

---

## Prerequisites

- Vivado 2023.1 or later (same version used in Vitis HLS export)
- VCK190 board files installed in Vivado
- Three exported HLS IP folders, each containing `component.xml`:
  - `ip_repo/rdbs_kernel/`
  - `ip_repo/covariance_estimation/`
  - `ip_repo/mvdr_weights/`

---

---

# PART 1 — PROJECT SETUP

---

## Step 1 — Create the Vivado Project

1. Launch Vivado.
2. In the Quick Start panel, click **Create Project**.
3. Click **Next**.
4. In the **Project Name** field, type: `mvdr_system`
5. In the **Project Location** field, browse to your working directory (e.g. `~/vivado_projects/`).
6. Leave **Create project subdirectory** checked.
7. Click **Next**.
8. On the Project Type page, select **RTL Project**.
9. Check **Do not specify sources at this time**.
10. Click **Next**.
11. On the Default Part page, click the **Boards** tab at the top.
12. In the search box, type `VCK190`.
13. Select **Versal VCK190 Evaluation Platform** from the list.
14. Click **Next**.
15. Review the summary and click **Finish**.

Vivado opens the project. The Flow Navigator panel appears on the left.

---

## Step 2 — Add Your HLS IP Repositories

1. In the top menu bar, click **Tools → Settings**.
2. In the left tree of the Settings dialog, expand **IP** and click **Repository**.
3. In the right panel, click the **+** (plus) button under "IP Repositories".
4. Browse to the **parent folder** that contains all three IP subfolders (e.g. `~/my_project/ip_repo/`).
5. Click **Select**.
6. Vivado scans the folder. You should see a popup confirming it found **3 IPs**:
   - `rdbs_kernel_v1_0`
   - `covariance_estimation_v1_0`
   - `mvdr_weights_v1_0`
7. Click **OK** to close the scan popup.
8. Click **OK** to close the Settings dialog.

> **If Vivado reports 0 IPs found:** Each IP subfolder must directly contain the `component.xml` file. Check that your folder structure is `ip_repo/rdbs_kernel/component.xml` and not an extra level deep like `ip_repo/rdbs_kernel/solution1/impl/ip/component.xml`.

---

---

# PART 2 — CREATE THE BLOCK DESIGN

---

## Step 3 — Open the Block Design Editor

1. In the Flow Navigator (left panel), under **IP INTEGRATOR**, click **Create Block Design**.
2. In the dialog that appears:
   - **Design name**: type `mvdr_bd`
   - **Directory**: leave as `<Local to Project>`
   - **Specify source set**: leave as `Design Sources`
3. Click **OK**.
4. The blank block design canvas opens.

---

---

# PART 3 — ADD ALL IP BLOCKS

Add IPs in this exact order. For each one: right-click on the empty canvas → **Add IP** → type the name in the search box → double-click the result.

---

## Step 4 — Add CIPS (Control, Interfaces and Processing System)

1. Right-click canvas → **Add IP**.
2. Search: `CIPS`
3. Double-click **Control, Interfaces and Processing System**.
4. The block `versal_cips_0` appears on the canvas.
5. **Do not double-click to configure yet** — configure it in Step 11.

---

## Step 5 — Add AXI NoC

1. Right-click canvas → **Add IP**.
2. Search: `AXI NoC`
3. Double-click **AXI NoC**.
4. The block `axi_noc_0` appears on the canvas.
5. **Do not configure yet** — configure it in Step 12.

---

## Step 6 — Add Clocking Wizard

1. Right-click canvas → **Add IP**.
2. Search: `Clocking Wizard`
3. Double-click **Clocking Wizard**.
4. The block `clk_wiz_0` appears on the canvas.
5. **Do not configure yet** — configure it in Step 13.

---

## Step 7 — Add Processor System Reset

1. Right-click canvas → **Add IP**.
2. Search: `Processor System Reset`
3. Double-click **Processor System Reset**.
4. The block `proc_sys_reset_0` appears on the canvas.
5. No configuration needed — default settings are correct.

---

## Step 8 — Add AXI SmartConnect (for AXI-Lite control)

1. Right-click canvas → **Add IP**.
2. Search: `AXI SmartConnect`
3. Double-click **AXI SmartConnect**.
4. The block `smartconnect_0` appears on the canvas.
5. **Do not configure yet** — configure it in Step 14.

---

## Step 9 — Add a Second AXI SmartConnect (for MVDR DDR ports)

1. Right-click canvas → **Add IP**.
2. Search: `AXI SmartConnect`
3. Double-click **AXI SmartConnect** again.
4. A second block `smartconnect_1` appears on the canvas.
5. **Do not configure yet** — configure it in Step 15.

---

## Step 10 — Add Your Three HLS IPs

**10a — Add rdbs_kernel:**
1. Right-click canvas → **Add IP**.
2. Search: `rdbs_kernel`
3. Double-click it. Block `rdbs_kernel_0` appears.

**10b — Add covariance_estimation:**
1. Right-click canvas → **Add IP**.
2. Search: `covariance_estimation`
3. Double-click it. Block `covariance_estimation_0` appears.

**10c — Add mvdr_weights:**
1. Right-click canvas → **Add IP**.
2. Search: `mvdr_weights`
3. Double-click it. Block `mvdr_weights_0` appears.

You now have **8 blocks** on the canvas total.

---

---

# PART 4 — CONFIGURE EACH IP BLOCK

---

## Step 11 — Configure CIPS

1. Double-click `versal_cips_0` to open its configuration.
2. Click the **Board** tab at the very top of the dialog.
3. Click **Apply Board Preset** button — this auto-fills LPDDR4 settings for VCK190.
4. Click the **PS PMC** tab.
5. In the left tree, click **PS-PMC → PS LPD → LPD AXI NOC**.
   - Enable **LPD_AXI_NOC_0** — this gives the PS a path to your IPs via AXI-Lite.
6. In the left tree, click **PS-PMC → PS LPD → LPD_AXI**.
   - Enable **M_AXI_LPD** — this is the PS master port you'll connect to your SmartConnect.
7. In the left tree, click **PS-PMC → Clocking → Output Clocks**.
   - Ensure **pl0_ref_clk** is enabled and set to **200 MHz**.
   - This clock feeds your Clocking Wizard.
8. Click **OK**.

---

## Step 12 — Configure AXI NoC

Double-click `axi_noc_0`. Go through each tab:

### General tab

| Field | Value |
|---|---|
| Number of AXI Slave Interfaces | **4** |
| Number of AXI Master Interfaces | **1** |
| Number of AXI Clocks | **2** |
| Number of Inter-NOC Slave Interfaces | 0 |
| Number of Inter-NOC Master Interfaces | 0 |
| Memory Controller | Single Memory Controller |
| Number of Memory Controller Ports | 1 |
| DDR Address Region 0 | DDR LOW0 (0x0000 0000 0000 to 0x0000 7FFF FFFF; 2G) |
| DDR Address Region 1 | NONE |

### Inputs tab

For each slave port, set the **Associated Clock** column:

| Slave Port | Associated Clock |
|---|---|
| S00_AXI | aclk0 |
| S01_AXI | aclk0 |
| S02_AXI | aclk0 |
| S03_AXI | aclk0 |

Leave all other fields (Read Bandwidth, Write Bandwidth, Outstanding Transactions) at **0** (unconstrained).

### Outputs tab

| Master Port | Associated Clock |
|---|---|
| M00_AXI | aclk0 |

### Connectivity tab

You will see a grid. Tick **every checkbox** in the S00–S03 rows under the M00_AXI column:

- S00_AXI → M00_AXI ✓ (tick this)
- S01_AXI → M00_AXI ✓ (tick this)
- S02_AXI → M00_AXI ✓ (tick this)
- S03_AXI → M00_AXI ✓ (tick this)

### QoS tab

For each slave port, set:

| Port | Traffic Class | Read Bandwidth | Write Bandwidth |
|---|---|---|---|
| S00_AXI | BEST_EFFORT | 0 | 0 |
| S01_AXI | BEST_EFFORT | 0 | 0 |
| S02_AXI | BEST_EFFORT | 0 | 0 |
| S03_AXI | BEST_EFFORT | 0 | 0 |

### DDR Basic tab

| Field | Value |
|---|---|
| Controller Type | **LPDDR4 SDRAM** |
| Clock selection | Memory Clock |
| Memory Clock period (ps) | **625** (= 1600 MHz) |
| Input System Clock Period (ps) | **10000** (= 100 MHz) |
| System Clock | **Differential** |
| Enable Internal Responder | **unchecked** |

### DDR Memory tab

| Field | Value |
|---|---|
| Device Type | Components |
| Speed Bin | LPDDR4-3200 |
| Base Component Width | x32 |
| Memory Device Width | x32 |
| Row Address Width | 17 |
| Bank Address Width | 3 |
| Bank Group Width | 0 |
| Number of Channels | Single |
| Data Width per Channel | 32 |
| Ranks | 1 |
| Stack Height | 1 |
| ECC | unchecked |
| Write DBI | DM NO DBI |
| Read DBI | unchecked |

Click **OK** to close the NoC config.

---

## Step 13 — Configure Clocking Wizard

1. Double-click `clk_wiz_0`.
2. Click the **Clocking Options** tab:
   - **Primitive**: MMCM
   - **Input Frequency (MHz)**: `200` (coming from CIPS pl0_ref_clk)
   - **Input clock type**: Single ended
3. Click the **Output Clocks** tab:
   - **clk_out1**: Enable ✓, **Requested Output Freq (MHz)**: `300`
   - All other outputs: disabled
   - **Reset Type**: Active Low
   - **Enable Locked**: checked ✓ (exposes the `locked` output pin)
4. Click **OK**.

---

## Step 14 — Configure AXI SmartConnect 0 (AXI-Lite control fan-out)

1. Double-click `smartconnect_0`.
2. Set:
   - **Number of Slave Interfaces**: `1`
   - **Number of Master Interfaces**: `3`
3. Click **OK**.

This SmartConnect takes the single PS master port and fans it out to the three HLS IP control ports.

---

## Step 15 — Configure AXI SmartConnect 1 (MVDR DDR port merger)

1. Double-click `smartconnect_1`.
2. Set:
   - **Number of Slave Interfaces**: `2`
   - **Number of Master Interfaces**: `1`
3. Click **OK**.

This SmartConnect merges MVDR's `gmem1` (a_in) and `gmem2` (w_out) onto a single NoC slave port, since both are tiny (21-element) transfers.

---

---

# PART 5 — WIRE ALL CONNECTIONS

Work through connections in this exact order. To connect: hover over a port on a block until you see a green pencil cursor, then click and drag to the destination port. A green line indicates a valid connection.

---

## Step 16 — Clock Connections

### 16a — CIPS clock output → Clocking Wizard input

| From (block : port) | To (block : port) |
|---|---|
| `versal_cips_0` : `pl0_ref_clk` | `clk_wiz_0` : `clk_in1` |

### 16b — CIPS reset → Clocking Wizard reset

| From | To |
|---|---|
| `versal_cips_0` : `pl0_resetn` | `clk_wiz_0` : `resetn` |

### 16c — Clocking Wizard 300 MHz output → all consumers

Connect `clk_wiz_0 : clk_out1` to **all** of the following (one by one):

| Destination block | Destination port |
|---|---|
| `proc_sys_reset_0` | `slowest_sync_clk` |
| `smartconnect_0` | `aclk` |
| `smartconnect_1` | `aclk` |
| `rdbs_kernel_0` | `ap_clk` |
| `covariance_estimation_0` | `ap_clk` |
| `mvdr_weights_0` | `ap_clk` |
| `axi_noc_0` | `aclk0` |
| `axi_noc_0` | `aclk1` |

> **How to connect one source to many destinations:** Connect the first one by dragging. For each subsequent destination, hover over the existing wire, right-click → **Add Junction**, then drag from the junction to the new destination. Alternatively, Vivado allows you to click the source port repeatedly.

---

## Step 17 — Reset Connections

### 17a — CIPS reset → Processor System Reset

| From | To |
|---|---|
| `versal_cips_0` : `pl0_resetn` | `proc_sys_reset_0` : `ext_reset_in` |

> Note: `pl0_resetn` connects to BOTH `clk_wiz_0 resetn` (Step 16b) and `proc_sys_reset_0 ext_reset_in`. Right-click the existing wire and add a junction.

### 17b — Clocking Wizard locked → Processor System Reset

| From | To |
|---|---|
| `clk_wiz_0` : `locked` | `proc_sys_reset_0` : `dcm_locked` |

### 17c — Processor System Reset outputs → all HLS IPs

Connect `proc_sys_reset_0 : peripheral_aresetn[0:0]` to **all** of the following:

| Destination block | Destination port |
|---|---|
| `rdbs_kernel_0` | `ap_rst_n` |
| `covariance_estimation_0` | `ap_rst_n` |
| `mvdr_weights_0` | `ap_rst_n` |
| `smartconnect_0` | `aresetn` |
| `smartconnect_1` | `aresetn` |

---

## Step 18 — AXI-Stream Connection (RDBS → Covariance)

This is the data pipeline between your first two IPs.

| From | To |
|---|---|
| `rdbs_kernel_0` : `y_out` | `covariance_estimation_0` : `y_in` |

Both ports are 32-bit AXI-Stream with TLAST. The connection is direct — no FIFO needed.

> **Verify the connection:** After connecting, click the wire. In the properties panel at the bottom, confirm:
> - Protocol: AXI4Stream
> - Data Width: 32
> - Clock: both sides at same `ap_clk`

---

## Step 19 — AXI-Lite Control Connections (PS → IPs)

### 19a — CIPS master → SmartConnect 0 slave

| From | To |
|---|---|
| `versal_cips_0` : `M_AXI_LPD` | `smartconnect_0` : `S00_AXI` |

### 19b — SmartConnect 0 masters → HLS IP control ports

| From | To |
|---|---|
| `smartconnect_0` : `M00_AXI` | `rdbs_kernel_0` : `s_axi_ctrl` |
| `smartconnect_0` : `M01_AXI` | `covariance_estimation_0` : `s_axi_ctrl` |
| `smartconnect_0` : `M02_AXI` | `mvdr_weights_0` : `s_axi_ctrl` |

> **Port name note:** The AXI-Lite control port from Vitis HLS is named based on the `bundle=ctrl` pragma. It will appear as `s_axi_ctrl` on each block. If your HLS pragmas used a different bundle name, the port name will differ — check the port list by clicking the block.

---

## Step 20 — AXI4 Master Connections (IPs → NoC → DDR)

These carry the actual data burst transfers to/from LPDDR4.

### 20a — Covariance AXI4 master → NoC S01

| From | To |
|---|---|
| `covariance_estimation_0` : `m_axi_gmem` | `axi_noc_0` : `S01_AXI` |

### 20b — MVDR gmem0 (R_in read) → NoC S02

| From | To |
|---|---|
| `mvdr_weights_0` : `m_axi_gmem0` | `axi_noc_0` : `S02_AXI` |

### 20c — MVDR gmem1 and gmem2 → SmartConnect 1 → NoC S03

MVDR has two small AXI4 masters (`gmem1` for a_in, `gmem2` for w_out). Merge them through SmartConnect 1:

| From | To |
|---|---|
| `mvdr_weights_0` : `m_axi_gmem1` | `smartconnect_1` : `S00_AXI` |
| `mvdr_weights_0` : `m_axi_gmem2` | `smartconnect_1` : `S01_AXI` |
| `smartconnect_1` : `M00_AXI` | `axi_noc_0` : `S03_AXI` |

### 20d — CIPS DDR access → NoC S00

The PS also needs DDR access (to write input data and read output weights from software):

| From | To |
|---|---|
| `versal_cips_0` : `NOC_LPD_AXI_0` | `axi_noc_0` : `S00_AXI` |

> **Note:** The CIPS NoC port name may appear as `NOC_LPD_AXI_0` or `LPD_AXI_NOC_0` depending on Vivado version. Connect whichever NoC-type port is exposed on the CIPS block.

---

## Step 21 — NoC DDR Clock (sys_clk0)

The NoC's `sys_clk0` needs the board's LPDDR4 differential reference clock. You need a differential input buffer IP.

### 21a — Add Utility Buffer IP

1. Right-click canvas → **Add IP**.
2. Search: `Utility Buffer`
3. Double-click **Utility Buffer**.
4. Block `util_ds_buf_0` appears.
5. Double-click `util_ds_buf_0` to configure:
   - **C Buf Type**: `IBUFDS`
6. Click **OK**.

### 21b — Make the LPDDR4 clock an external port

1. Right-click `util_ds_buf_0 : IBUF_DS_P` → **Make External**.
   - A top-level port named `IBUF_DS_P` is created.
   - Rename it: click the port, press F2, type `lpddr4_clk_p`
2. Right-click `util_ds_buf_0 : IBUF_DS_N` → **Make External**.
   - Rename it to `lpddr4_clk_n`

### 21c — Connect buffer output to NoC

| From | To |
|---|---|
| `util_ds_buf_0` : `IBUF_OUT` | `axi_noc_0` : `sys_clk0` |

---

## Step 22 — Make RDBS Input an External Port

The `rdbs_kernel_0 : x_in` port receives raw ADC/DMA data. For now, make it an external AXI-Stream port so you can drive it from a DMA or testbench:

1. Right-click `rdbs_kernel_0 : x_in` → **Make External**.
2. A port named `x_in` appears at the canvas edge.
3. This will be connected to a DMA IP or driven from the PS via AXI-DMA in future.

> **Alternative:** If you have an AXI DMA IP for the ADC source, add it now and connect its `M_AXIS_MM2S` → `rdbs_kernel_0 : x_in`. That DMA would also need its own AXI-Lite control connection from the SmartConnect (add a 4th master port to `smartconnect_0`) and an AXI4 master connection to the NoC for reading from DDR.

---

---

# PART 6 — ADDRESS EDITOR

---

## Step 23 — Assign Addresses

1. In the block design, click the **Address Editor** tab (next to the Diagram tab at the top).
2. You will see two sections: **Data** (AXI4 master address spaces) and **AXI-Lite** (control register spaces).

### 23a — AXI-Lite control register addresses

Expand the PS master path. Assign these addresses to the three HLS IP control ports:

| IP Control Port | Base Address | Size |
|---|---|---|
| `rdbs_kernel_0 / s_axi_ctrl` | `0xA000_0000` | 4K |
| `covariance_estimation_0 / s_axi_ctrl` | `0xA000_1000` | 4K |
| `mvdr_weights_0 / s_axi_ctrl` | `0xA000_2000` | 4K |

To assign: right-click each entry → **Assign Address**, then type the base address manually, or click **Assign All** to let Vivado auto-assign.

### 23b — AXI4 master DDR addresses

All AXI4 master ports from the HLS IPs must be mapped to the DDR range. Expand each master:

| Master | Segment | Base Address | Size |
|---|---|---|---|
| `covariance_estimation_0 / m_axi_gmem` | DDR LOW0 | `0x0000_0000` | 2G |
| `mvdr_weights_0 / m_axi_gmem0` | DDR LOW0 | `0x0000_0000` | 2G |
| `mvdr_weights_0 / m_axi_gmem1` | DDR LOW0 | `0x0000_0000` | 2G |
| `mvdr_weights_0 / m_axi_gmem2` | DDR LOW0 | `0x0000_0000` | 2G |

> All four can map to the same DDR segment — the actual buffer locations within DDR are set at runtime by the PS software writing addresses into the AXI-Lite control registers.

Click **Assign All** if any addresses remain unassigned (shown with a yellow warning icon).

---

---

# PART 7 — VALIDATE AND GENERATE

---

## Step 24 — Validate the Block Design

1. In the block design toolbar, click the **Validate Design** button (green checkmark icon), or press **F6**.
2. Vivado runs DRC checks. Wait for it to complete.
3. **Expected result**: "Validation successful — No errors or critical warnings."

### Common errors and fixes

| Error message | Fix |
|---|---|
| "Clock domain crossing detected between X and Y" | Make sure all ap_clk inputs are connected to the same clk_wiz_0 clk_out1 wire |
| "AXI protocol mismatch on port X" | Check that AXI4 master ports are connected to AXI NoC slaves, not to SmartConnect; AXI-Lite ports connect to SmartConnect only |
| "NoC slave S0X has no clock assigned" | Go back to NoC config → Inputs tab → set Associated Clock for that slave to aclk0 |
| "No path from master to memory" | Go to NoC Connectivity tab → tick the missing checkbox for that slave |
| "proc_sys_reset dcm_locked not connected" | Connect clk_wiz_0 locked → proc_sys_reset_0 dcm_locked |
| "Address not assigned for master X" | Open Address Editor → right-click unassigned entry → Assign Address |

---

## Step 25 — Create HDL Wrapper

1. In the **Sources** panel (left side), find `mvdr_bd.bd` under Design Sources.
2. Right-click `mvdr_bd.bd` → **Create HDL Wrapper**.
3. In the dialog, select **Let Vivado manage wrapper and auto-update**.
4. Click **OK**.
5. Vivado generates `mvdr_bd_wrapper.v` and it appears as the top-level source.

---

## Step 26 — Add XDC Pin Constraints

1. In the Flow Navigator, click **Add Sources** (or File → Add Sources).
2. Select **Add or Create Constraints** → Next.
3. Click **Create File** → File type: XDC → File name: `vck190_pins` → OK → Finish.
4. In the Sources panel, double-click `vck190_pins.xdc` to open it.
5. Add the following constraints:

```tcl
# =============================================================
# VCK190 LPDDR4 Reference Clock (100 MHz differential)
# Bank 66 — verify exact pins from VCK190 board schematic
# =============================================================
set_property PACKAGE_PIN BH51 [get_ports {lpddr4_clk_p}]
set_property PACKAGE_PIN BJ51 [get_ports {lpddr4_clk_n}]
set_property IOSTANDARD LVDS [get_ports {lpddr4_clk_p}]
set_property IOSTANDARD LVDS [get_ports {lpddr4_clk_n}]

# =============================================================
# False path on reset (crosses clock domains intentionally)
# =============================================================
set_false_path -from [get_ports pl0_resetn]

# =============================================================
# Timing constraint for PL clock (300 MHz = 3.333 ns period)
# Vivado auto-creates this from the Clocking Wizard but add
# it explicitly if timing reports show an unconstrained clock
# =============================================================
# create_clock -period 3.333 -name pl_clk300 [get_pins clk_wiz_0/clk_out1]
```

> **Important:** The exact PACKAGE_PIN values for the LPDDR4 clock on the VCK190 are defined in Xilinx's official VCK190 XDC file. After installing board files, find it at:
> `<Vivado_install>/data/boards/board_files/vck190/*/vck190.xdc`
> Copy the LPDDR4 clock pin entries from that file into yours.

6. Save the XDC file (Ctrl+S).

---

## Step 27 — Run Synthesis

1. In Flow Navigator, click **Run Synthesis**.
2. In the Launch Runs dialog, leave settings at default.
3. Click **OK**.
4. Wait for synthesis to complete (15–30 minutes).
5. When prompted, click **Open Synthesized Design** to review, or click **Run Implementation** directly.

**Check synthesis report:**
- In the Synthesis Run results, open **utilization report**
- Your three small IPs should use well under 10% of VCK190 resources

---

## Step 28 — Run Implementation

1. Click **Run Implementation**.
2. Click **OK** in the Launch Runs dialog.
3. Wait for implementation to complete (30–90 minutes for Versal).
4. When complete, open **Implemented Design** to review timing.

**Check timing report:**
- Open **Reports → Timing → Check Timing**
- All clocks must show **WNS (Worst Negative Slack) ≥ 0**
- If `ap_clk` at 300 MHz fails timing (WNS < 0), reduce to 250 MHz:
  - Double-click `clk_wiz_0` → Output Clocks tab → change clk_out1 to 250 MHz
  - Re-run implementation

---

## Step 29 — Generate Bitstream

1. Click **Generate Bitstream**.
2. Click **OK**.
3. Wait for completion (10–20 minutes).
4. When done, click **Cancel** on the "Open Implemented Design" prompt (not needed now).

---

## Step 30 — Export Hardware Platform

1. File → **Export → Export Hardware**.
2. In the dialog:
   - **Include bitstream**: ✓ checked
   - **XSA file name**: `mvdr_system`
   - **Export to**: choose your project directory
3. Click **OK** (or **Finish** in newer Vivado versions).
4. The file `mvdr_system.xsa` is generated.

This XSA file is used in Vitis to create the software platform and write your PS application.

---

---

# PART 8 — RUNTIME SOFTWARE FLOW

---

## Step 31 — How the PS Controls Everything at Runtime

This is what your bare-metal or Linux software must do in sequence:

### AXI-Lite register map for each kernel

The HLS tools generate a control register at the `s_axi_ctrl` base address:

| Offset | Register | Description |
|---|---|---|
| 0x00 | `ap_ctrl` | bit 0 = ap_start, bit 1 = ap_done, bit 2 = ap_idle |
| 0x10 | `R` or `R_in` (lower 32 bits) | DDR address of R matrix buffer |
| 0x14 | `R` or `R_in` (upper 32 bits) | Upper 32 bits of address (usually 0) |
| 0x18 | `a_in` (lower 32 bits) | DDR address of steering vector buffer |
| 0x20 | `w_out` (lower 32 bits) | DDR address of weights output buffer |

> Exact register offsets are in the Vitis HLS synthesis report under **Interface → AXI-Lite slave register map**. Check `solution1/impl/report/verilog/export_syn.rpt` for each kernel.

### Execution sequence

```
Step 1: Allocate DDR buffers (from PS software):
        R_buf   at 0x0010_0000  (21×21 × 8 bytes = 3528 bytes)
        a_buf   at 0x0010_2000  (21 × 8 bytes = 168 bytes)
        w_buf   at 0x0010_3000  (21 × 8 bytes = 168 bytes)

Step 2: Write covariance_estimation control registers:
        Write 0x0010_0000 to address 0xA000_1010  (R buffer address)
        Write 0x00000001 to address 0xA000_1000   (ap_start = 1)

Step 3: Write rdbs_kernel control registers:
        Write 1000 (K_SNAPSHOTS) to the num_snapshots register
        Write 0x00000001 to address 0xA000_0000   (ap_start = 1)
        → rdbs starts streaming, covariance receives via AXI-Stream

Step 4: Poll covariance_estimation ap_done:
        Loop: read address 0xA000_1000, check bit 1 = 1

Step 5: Write mvdr_weights control registers:
        Write 0x0010_0000 to address 0xA000_2010  (R_in address)
        Write 0x0010_2000 to address 0xA000_2018  (a_in address)
        Write 0x0010_3000 to address 0xA000_2020  (w_out address)
        Write 0x00000001 to address 0xA000_2000   (ap_start = 1)

Step 6: Poll mvdr_weights ap_done:
        Loop: read address 0xA000_2000, check bit 1 = 1

Step 7: Read w_buf from DDR at 0x0010_3000
        → 21 complex weights (ap_fixed<32,9> pairs)
```

---

---

# APPENDIX — Complete Connection Reference Table

All connections in the design, in one place:

## Clock and Reset Connections

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| C1 | versal_cips_0 | pl0_ref_clk | clk_wiz_0 | clk_in1 |
| C2 | versal_cips_0 | pl0_resetn | clk_wiz_0 | resetn |
| C3 | versal_cips_0 | pl0_resetn | proc_sys_reset_0 | ext_reset_in |
| C4 | clk_wiz_0 | clk_out1 | proc_sys_reset_0 | slowest_sync_clk |
| C5 | clk_wiz_0 | clk_out1 | smartconnect_0 | aclk |
| C6 | clk_wiz_0 | clk_out1 | smartconnect_1 | aclk |
| C7 | clk_wiz_0 | clk_out1 | rdbs_kernel_0 | ap_clk |
| C8 | clk_wiz_0 | clk_out1 | covariance_estimation_0 | ap_clk |
| C9 | clk_wiz_0 | clk_out1 | mvdr_weights_0 | ap_clk |
| C10 | clk_wiz_0 | clk_out1 | axi_noc_0 | aclk0 |
| C11 | clk_wiz_0 | clk_out1 | axi_noc_0 | aclk1 |
| C12 | clk_wiz_0 | locked | proc_sys_reset_0 | dcm_locked |
| C13 | proc_sys_reset_0 | peripheral_aresetn[0:0] | rdbs_kernel_0 | ap_rst_n |
| C14 | proc_sys_reset_0 | peripheral_aresetn[0:0] | covariance_estimation_0 | ap_rst_n |
| C15 | proc_sys_reset_0 | peripheral_aresetn[0:0] | mvdr_weights_0 | ap_rst_n |
| C16 | proc_sys_reset_0 | peripheral_aresetn[0:0] | smartconnect_0 | aresetn |
| C17 | proc_sys_reset_0 | peripheral_aresetn[0:0] | smartconnect_1 | aresetn |

## Data Path Connections

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| D1 | rdbs_kernel_0 | y_out | covariance_estimation_0 | y_in | AXI-Stream |
| D2 | versal_cips_0 | NOC_LPD_AXI_0 | axi_noc_0 | S00_AXI | AXI4 |
| D3 | covariance_estimation_0 | m_axi_gmem | axi_noc_0 | S01_AXI | AXI4 |
| D4 | mvdr_weights_0 | m_axi_gmem0 | axi_noc_0 | S02_AXI | AXI4 |
| D5 | mvdr_weights_0 | m_axi_gmem1 | smartconnect_1 | S00_AXI | AXI4 |
| D6 | mvdr_weights_0 | m_axi_gmem2 | smartconnect_1 | S01_AXI | AXI4 |
| D7 | smartconnect_1 | M00_AXI | axi_noc_0 | S03_AXI | AXI4 |

## Control Connections (AXI-Lite)

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| L1 | versal_cips_0 | M_AXI_LPD | smartconnect_0 | S00_AXI | AXI-Lite |
| L2 | smartconnect_0 | M00_AXI | rdbs_kernel_0 | s_axi_ctrl | AXI-Lite |
| L3 | smartconnect_0 | M01_AXI | covariance_estimation_0 | s_axi_ctrl | AXI-Lite |
| L4 | smartconnect_0 | M02_AXI | mvdr_weights_0 | s_axi_ctrl | AXI-Lite |

## DDR Clock Connections

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| K1 | util_ds_buf_0 | IBUF_OUT | axi_noc_0 | sys_clk0 |
| K2 | (external port) | lpddr4_clk_p | util_ds_buf_0 | IBUF_DS_P |
| K3 | (external port) | lpddr4_clk_n | util_ds_buf_0 | IBUF_DS_N |

## External Ports

| Port Name | Direction | Connected To | Purpose |
|---|---|---|---|
| `lpddr4_clk_p` | Input | util_ds_buf_0 IBUF_DS_P | LPDDR4 diff ref clock + |
| `lpddr4_clk_n` | Input | util_ds_buf_0 IBUF_DS_N | LPDDR4 diff ref clock − |
| `x_in` (AXI-Stream) | Input | rdbs_kernel_0 x_in | Raw ADC / DMA input |

---

## Summary Checklist

Before running synthesis, confirm every item:

- [ ] All 3 HLS IPs added from ip_repo
- [ ] CIPS board preset applied (LPDDR4)
- [ ] AXI NoC: 4 slaves, 1 master, 2 clocks
- [ ] AXI NoC DDR Basic: LPDDR4 SDRAM, 625 ps, 10000 ps sys clk, Differential
- [ ] AXI NoC Connectivity: all 4 slave-to-master boxes ticked
- [ ] AXI NoC QoS: all 4 ports set to BEST_EFFORT
- [ ] Clocking Wizard: 200 MHz in → 300 MHz out, active-low reset
- [ ] SmartConnect 0: 1 slave, 3 masters (AXI-Lite fan-out)
- [ ] SmartConnect 1: 2 slaves, 1 master (MVDR DDR merger)
- [ ] All 17 clock/reset connections made (C1–C17)
- [ ] All 7 data path connections made (D1–D7)
- [ ] All 4 AXI-Lite control connections made (L1–L4)
- [ ] All 3 DDR clock connections made (K1–K3)
- [ ] util_ds_buf_0 configured as IBUFDS
- [ ] External ports created for lpddr4_clk_p/n and x_in
- [ ] Address Editor: AXI-Lite addresses assigned (0xA000_0000, 1000, 2000)
- [ ] Address Editor: AXI4 masters mapped to DDR LOW0
- [ ] Design validated (F6) — zero errors
- [ ] HDL wrapper created
- [ ] XDC file added with LPDDR4 clock pin constraints
