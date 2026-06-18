      # VCK190 Vivado 2025.2 — Complete IP Integration Guide
## RDBS Kernel + Covariance Estimation + MVDR Weights

---

> **Read this first.**
> This guide is written specifically for **Vivado 2025.2** on the **VCK190** board.
> Every menu name, button label, tab name, and port name matches what you see on screen in 2025.2.
> Do every step in the exact order written. Do not skip any step.

---

## System Architecture (What You Are Building)

```
[External ADC / DMA source]
         |
         | AXI-Stream  (64 complex samples per snapshot)
         v
  [rdbs_kernel_0]          ← controlled by PS via AXI-Lite
         |
         | AXI-Stream  (21 complex beams per snapshot)
         v
  [covariance_estimation_0] ← controlled by PS via AXI-Lite
         |
         | AXI4 Master (writes 21×21 R matrix to LPDDR4)
         v
     [AXI NoC] ---------> [LPDDR4 DDR]
         ^
         | AXI4 Master (reads R matrix and steering vector a from LPDDR4,
         |              writes weights w back to LPDDR4)
  [mvdr_weights_0]         ← controlled by PS via AXI-Lite
         ^
         |  AXI-Lite (control registers for all 3 IPs)
  [AXI SmartConnect]
         ^
         |
  [CIPS / APU / PS]
```

---

---

# PART 1 — PROJECT CREATION

---

## STEP 1 — Launch Vivado 2025.2

1. Open Vivado 2025.2 from your application menu or terminal.
2. Wait for the Welcome screen to fully load.
3. Confirm the title bar reads **Vivado 2025.2** before proceeding.

---

## STEP 2 — Create a New Project

1. On the Welcome screen, click **Create Project**.
   > If the Welcome screen is not visible: click **File → Project → New…**
2. The **New Project** wizard opens. Click **Next**.
3. On the **Project Name** page:
   - **Project name** field: type `mvdr_system`
   - **Project location** field: click the `…` browse button and choose a directory on a drive with at least 20 GB free (e.g. `D:/vivado_projects/`).
   - Leave **Create project subdirectory** ticked.
   - Click **Next**.
4. On the **Project Type** page:
   - Select **RTL Project**.
   - Tick **Do not specify sources at this time**.
   - Leave **Project is an extensible Vitis platform** UNTICKED (you don't need Vitis platform flow).
   - Click **Next**.
5. On the **Default Part** page:
   - Click the **Boards** tab at the top (not the Parts tab).
   - In the **Search** box type: `VCK190`
   - In the results list, click to select **Versal VCK190 Evaluation Platform**.
   - Confirm the **Board Rev** column shows the revision of your physical board (usually `1.0` or `2.0`).
   - Click **Next**.
6. On the **New Project Summary** page, review and click **Finish**.
7. Vivado creates the project and opens the **Project Manager** view.

---

## STEP 3 — Add Your HLS IP Repositories

Your three Vitis HLS–exported IPs must be registered before you can add them to a block design.

1. In the top menu bar, click **Tools → Settings**.
2. The Settings dialog opens. In the left-side tree, click **IP → Repository**.
3. The right panel shows "IP Repositories". Click the **blue + button** (Add Repository).
4. A file browser opens. Navigate to the **parent folder** that contains all three of your HLS IP export folders. For example, if your IPs are at:
   ```
   C:/my_project/ip_repo/rdbs_kernel/
   C:/my_project/ip_repo/covariance_estimation/
   C:/my_project/ip_repo/mvdr_weights/
   ```
   Then navigate to `C:/my_project/ip_repo/` and click **Select**.
5. Vivado scans all subfolders. A small popup window appears listing the discovered IPs.
   - You must see exactly **3 IPs** listed:
     - `rdbs_kernel_v1_0` (or similar versioned name)
     - `covariance_estimation_v1_0`
     - `mvdr_weights_v1_0`
   - If you see 0 IPs, it means the folder structure is wrong. Each HLS IP folder must directly contain a `component.xml` file. Open a file browser and verify: `ip_repo/rdbs_kernel/component.xml` must exist. If it is buried deeper (e.g. `solution1/impl/ip/`), copy the `impl/ip/` folder contents up to `ip_repo/rdbs_kernel/`.
6. Click **OK** on the IP discovery popup.
7. Click **OK** on the Settings dialog.

---

---

# PART 2 — CREATE THE BLOCK DESIGN

---

## STEP 4 — Open Block Design Editor

1. In the **Flow Navigator** panel on the left side of Vivado, look for the section called **IP INTEGRATOR**.
2. Click **Create Block Design**.
3. A small dialog box appears:
   - **Design name**: type `mvdr_bd`
   - **Directory**: leave as `<Local to Project>`
   - **Specify source set**: leave as `Design Sources`
4. Click **OK**.
5. A blank white canvas opens. This is your block design editor. The tab at the top is labeled **Diagram**.

> **Save often.** Press **Ctrl+S** after every few steps to save the block design.

---

---

# PART 3 — ADD ALL IPs TO THE CANVAS

Add IPs one at a time using this method:
- Right-click anywhere on the blank canvas.
- Click **Add IP**.
- An IP catalog search bar appears (either floating or in a panel).
- Type the name, wait for the result, double-click the result.
- The IP block appears on the canvas.

Add them in this exact order:

---

## STEP 5 — Add CIPS (Control, Interfaces and Processing System)

1. Right-click canvas → **Add IP**.
2. In the search box, type: `CIPS`
3. In the results, double-click **Control, Interfaces and Processing System**.
4. The block named `versal_cips_0` appears on the canvas.
5. **Do not configure it yet.** Leave it on the canvas.

---

## STEP 6 — Add AXI NoC

1. Right-click canvas → **Add IP**.
2. Type: `AXI NoC`
3. Double-click **AXI NoC**.
4. The block `axi_noc_0` appears on the canvas.
5. **Do not configure it yet.**

---

## STEP 7 — Add Clocking Wizard

1. Right-click canvas → **Add IP**.
2. Type: `Clocking Wizard`
3. Double-click **Clocking Wizard**.
4. Block `clk_wiz_0` appears. **Do not configure yet.**

---

## STEP 8 — Add Processor System Reset

1. Right-click canvas → **Add IP**.
2. Type: `Processor System Reset`
3. Double-click **Processor System Reset**.
4. Block `proc_sys_reset_0` appears. No configuration needed for this one — defaults are correct.

---

## STEP 9 — Add AXI SmartConnect (for AXI-Lite Control)

This SmartConnect fans out the PS master port to all three HLS IP control ports.

1. Right-click canvas → **Add IP**.
2. Type: `AXI SmartConnect`
3. Double-click **AXI SmartConnect**.
4. Block `smartconnect_0` appears. **Do not configure yet.**

---

## STEP 10 — Add a Second AXI SmartConnect (for MVDR DDR ports)

MVDR has three AXI4 master bundles (gmem0, gmem1, gmem2). This SmartConnect merges gmem1 and gmem2 into one NoC port.

1. Right-click canvas → **Add IP**.
2. Type: `AXI SmartConnect`
3. Double-click **AXI SmartConnect** again.
4. A second block `smartconnect_1` appears. **Do not configure yet.**

---

## STEP 11 — Add Your Three HLS IP Blocks

**11a — rdbs_kernel:**
1. Right-click canvas → **Add IP**.
2. Type: `rdbs_kernel`
3. Double-click it. Block `rdbs_kernel_0` appears.

**11b — covariance_estimation:**
1. Right-click canvas → **Add IP**.
2. Type: `covariance_estimation`
3. Double-click it. Block `covariance_estimation_0` appears.

**11c — mvdr_weights:**
1. Right-click canvas → **Add IP**.
2. Type: `mvdr_weights`
3. Double-click it. Block `mvdr_weights_0` appears.

---

## STEP 12 — Add Utility Buffer (for LPDDR4 differential clock)

The NoC needs the board's LPDDR4 reference clock through a differential input buffer.

1. Right-click canvas → **Add IP**.
2. Type: `Utility Buffer`
3. Double-click **Utility Buffer**.
4. Block `util_ds_buf_0` appears.

---

You now have **9 blocks** on the canvas. Press **Ctrl+S** to save.

---

---

# PART 4 — CONFIGURE EACH IP BLOCK

Double-click each block to open its configuration dialog.

---

## STEP 13 — Configure CIPS

1. Double-click `versal_cips_0`.
2. The CIPS configuration wizard opens. You will see a page with **Design Flow** at the top.
3. **Design Flow** dropdown: select **Full System**. Click **Next**.
4. The next page shows **PS PMC** on the left tree. Click on **PS PMC** to expand it.

### 13a — Enable M_AXI_FPD (PS master for AXI-Lite control)

5. In the left tree, click **PS-PMC → PS PL Interfaces**.
6. On the right, look for **AXI Master Interfaces**.
7. Find **M_AXI_FPD** and tick the checkbox to **Enable** it.
   - Leave Data Width at default (128 or 32 — either is fine).
8. Also find **Number of PL Resets** and set it to **1**.
   - This creates the `pl0_resetn` output on the CIPS block.

### 13b — Enable PS NoC connection (PS to DDR via NoC)

9. In the left tree, click **PS-PMC → PS PL Interfaces → PS NOC Interfaces**.
10. Enable **NOC_PS_CCI_0** (or **PS_NOC_LPD_AXI_0** depending on version).
    - This gives the PS processor direct access to LPDDR4 DDR for reading weights back from software.
    - If you see a list of NOC interfaces, enable the first one in the list.

### 13c — Configure PL clock

11. In the left tree, click **PS-PMC → Clocking**.
12. Click the **Output Clocks** tab on the right.
13. Expand **PMC Domain Clocks → PL Fabric Clocks**.
14. Find **PL0_REF_CLK** and:
    - Tick the **Enable** checkbox.
    - Set the frequency to **200.000 MHz**.
    - This is the clock that will feed your Clocking Wizard.
15. Click **Finish**.
16. A second **Finish** button may appear — click it again.
17. You are back on the canvas. The CIPS block now shows ports including `pl0_ref_clk` and `pl0_resetn`.

---

## STEP 14 — Configure AXI NoC

1. Double-click `axi_noc_0`.
2. The AXI NoC configuration dialog opens with multiple tabs across the top.

### General tab

Click the **General** tab.

| Setting | Value to set |
|---|---|
| Number of AXI Slave Interfaces | **4** |
| Number of AXI Master Interfaces | **1** |
| Number of AXI Clocks | **2** |
| Number of Inter-NOC Slave Interfaces | 0 |
| Number of Inter-NOC Master Interfaces | 0 |
| Memory Controller | **Single Memory Controller** |
| Number of Memory Controller Ports | **1** |
| DDR Address Region 0 | **DDR LOW0 (0x0000 0000 0000 to 0x0000 7FFF FFFF; 2G)** |
| DDR Address Region 1 | **NONE** |

### Inputs tab

Click the **Inputs** tab. You see a table with S00_AXI, S01_AXI, S02_AXI, S03_AXI rows.

For **each of the four slave ports**, find the **Associated Clock** column and set it to **aclk0**:

| Port | Set Associated Clock to |
|---|---|
| S00_AXI | aclk0 |
| S01_AXI | aclk0 |
| S02_AXI | aclk0 |
| S03_AXI | aclk0 |

Leave all bandwidth and outstanding transaction fields at **0**.

### Outputs tab

Click the **Outputs** tab.

| Port | Set Associated Clock to |
|---|---|
| M00_AXI | aclk0 |

### Connectivity tab

Click the **Connectivity** tab. You see a grid with slave ports as rows and master ports as columns.

**Tick ALL four checkboxes** in the M00_AXI column:

- Row S00_AXI, Column M00_AXI → **tick** ✓
- Row S01_AXI, Column M00_AXI → **tick** ✓
- Row S02_AXI, Column M00_AXI → **tick** ✓
- Row S03_AXI, Column M00_AXI → **tick** ✓

### QoS tab

Click the **QoS** tab.

For every slave port, set **Traffic Class** to **BEST_EFFORT**:

| Port | Traffic Class |
|---|---|
| S00_AXI | BEST_EFFORT |
| S01_AXI | BEST_EFFORT |
| S02_AXI | BEST_EFFORT |
| S03_AXI | BEST_EFFORT |

Leave Read Bandwidth, Write Bandwidth, and all other fields at 0.

### DDR Basic tab

Click the **DDR Basic** tab.

| Setting | Value |
|---|---|
| Controller Type | **LPDDR4 SDRAM** |
| Clock selection | **Memory Clock** |
| Memory Clock period (ps) | **625** ← type this number (= 1600 MHz) |
| Input System Clock Period (ps) | **10000** ← select from dropdown or type (= 100 MHz) |
| System Clock | **Differential** |
| Enable Internal Responder | **UNCHECKED** |

> If you see `509` in the Memory Clock field (as in your screenshot), clear it and type `625`.
> If the Input System Clock dropdown shows `9925`, change it to `10000`.

### DDR Memory tab

Click the **DDR Memory** tab.

| Setting | Value |
|---|---|
| Device Type | Components |
| Speed Bin | **LPDDR4-3200** |
| Base Component Width | **x32** |
| Memory Device Width | **x32** |
| Row Address Width | **17** |
| Bank Address Width | **3** |
| Bank Group Width | **0** |
| Number of Channels | Single |
| Data Width per Channel | **32** |
| Ranks | **1** |
| Stack Height | 1 |
| ECC | Unchecked |
| Write DBI | DM NO DBI |
| Read DBI | Unchecked |

3. Click **OK** to close the NoC configuration.

---

## STEP 15 — Configure Clocking Wizard

1. Double-click `clk_wiz_0`.
2. Click the **Clocking Options** tab:
   - **Primitive**: MMCM (default, leave it)    
   - **Input Clock Information → Primary → Input Frequency (MHz)**: type `200`
   - **Jitter Optimization**: Balanced (default)
3. Click the **Output Clocks** tab:
   - **clk_out1** row:
     - **Output Freq Requested (MHz)**: type `300`
     - **Used**: must be ticked ✓
   - Scroll down to the **Enable Optional Inputs/Outputs** section:
     - **reset**: tick this ✓ (creates the `reset` input pin, active HIGH by default)
     - **locked**: tick this ✓ (creates the `locked` output pin)
   - **Reset Type**: change to **Active Low** (so the input pin becomes `resetn`)
4. Click **OK**.

---

## STEP 16 — Configure AXI SmartConnect 0 (AXI-Lite fan-out)

1. Double-click `smartconnect_0`.
2. Set:
   - **Number of Slave Interfaces**: `1`
   - **Number of Master Interfaces**: `3`
3. Click **OK**.

---

## STEP 17 — Configure AXI SmartConnect 1 (MVDR DDR merger)

1. Double-click `smartconnect_1`.
2. Set:
   - **Number of Slave Interfaces**: `2`
   - **Number of Master Interfaces**: `1`
3. Click **OK**.

---

## STEP 18 — Configure Utility Buffer

1. Double-click `util_ds_buf_0`.
2. Set:
   - **C Buf Type**: `IBUFDS`
3. Click **OK**.

---

Press **Ctrl+S** to save.

---

---

# PART 5 — MAKE ALL WIRE CONNECTIONS

> **How to connect two ports in Vivado 2025.2:**
> - Hover your mouse over a port on a block until the cursor changes to a small pencil or crosshair icon.
> - Click and hold, then drag to the destination port. A green line shows a valid connection.
> - When both ports are compatible, the line turns green solid. Release the mouse button.
> - To connect one output to multiple inputs: make the first connection, then hover over the existing wire, right-click → **Add Junction**, then drag from the junction to the next destination.

Work through every connection in the tables below. Check off each one as you make it.

---

## STEP 19 — Clock Connections

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| C1 | `versal_cips_0` | `pl0_ref_clk` | `clk_wiz_0` | `clk_in1` |
| C2 | `clk_wiz_0` | `clk_out1` | `proc_sys_reset_0` | `slowest_sync_clk` |
| C3 | `clk_wiz_0` | `clk_out1` | `smartconnect_0` | `aclk` |
| C4 | `clk_wiz_0` | `clk_out1` | `smartconnect_1` | `aclk` |
| C5 | `clk_wiz_0` | `clk_out1` | `rdbs_kernel_0` | `ap_clk` |
| C6 | `clk_wiz_0` | `clk_out1` | `covariance_estimation_0` | `ap_clk` |
| C7 | `clk_wiz_0` | `clk_out1` | `mvdr_weights_0` | `ap_clk` |
| C8 | `clk_wiz_0` | `clk_out1` | `axi_noc_0` | `aclk0` |
| C9 | `clk_wiz_0` | `clk_out1` | `axi_noc_0` | `aclk1` |
| C10 | `clk_wiz_0` | `locked` | `proc_sys_reset_0` | `dcm_locked` |

> **C3 through C9 all come from the same source (`clk_wiz_0 : clk_out1`).**
> Make connection C2 first (drag from `clk_out1` to `slowest_sync_clk`).
> For C3 onward: hover over the green wire you just drew, right-click → **Add Junction**, then drag from the junction dot to the next destination.

---

## STEP 20 — Reset Connections

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| R1 | `versal_cips_0` | `pl0_resetn` | `clk_wiz_0` | `resetn` |
| R2 | `versal_cips_0` | `pl0_resetn` | `proc_sys_reset_0` | `ext_reset_in` |
| R3 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `rdbs_kernel_0` | `ap_rst_n` |
| R4 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `covariance_estimation_0` | `ap_rst_n` |
| R5 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `mvdr_weights_0` | `ap_rst_n` |
| R6 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `smartconnect_0` | `aresetn` |
| R7 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `smartconnect_1` | `aresetn` |

> **R1 and R2 both come from `pl0_resetn`.** Make R1 first, then right-click the wire → Add Junction → drag to `ext_reset_in` for R2.
> **R3 through R7 all come from `peripheral_aresetn[0:0]`.** Same technique — add junctions to fan out.

---

## STEP 21 — AXI-Stream Connection (RDBS → Covariance pipeline)

This is the main data flow between your first two kernels.

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| S1 | `rdbs_kernel_0` | `y_out` | `covariance_estimation_0` | `y_in` | AXI4-Stream |

> How to find the port: Click on `rdbs_kernel_0` to see its ports listed on the sides of the block. `y_out` is on the right side (master). `y_in` on `covariance_estimation_0` is on the left side (slave). Drag from `y_out` to `y_in` directly.

---

## STEP 22 — AXI-Lite Control Connections (PS → HLS IP control registers)

These connections let the PS processor start/stop each kernel and pass DDR buffer addresses.

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| L1 | `versal_cips_0` | `M_AXI_FPD` | `smartconnect_0` | `S00_AXI` | AXI4-Lite |
| L2 | `smartconnect_0` | `M00_AXI` | `rdbs_kernel_0` | `s_axi_ctrl` | AXI4-Lite |
| L3 | `smartconnect_0` | `M01_AXI` | `covariance_estimation_0` | `s_axi_ctrl` | AXI4-Lite |
| L4 | `smartconnect_0` | `M02_AXI` | `mvdr_weights_0` | `s_axi_ctrl` | AXI4-Lite |

> **Port name note:** In Vitis HLS, the `#pragma HLS INTERFACE s_axilite bundle=ctrl` pragma creates a port with the bundle name `ctrl`. In Vivado 2025.2, this port appears on the block as `s_axi_ctrl`. If your HLS code used a different bundle name, the port will have a different name. Click the block and look at all ports listed on its sides to find it.

> **M_AXI_FPD note:** On some CIPS versions in 2025.2, this port appears as `M_AXI_FPD` or just `M_AXI_LPD` depending on which interface you enabled in Step 13. The port you need is the one you enabled (M_AXI_FPD).

---

## STEP 23 — AXI4 Master Connections (HLS IPs → NoC → LPDDR4)

These carry burst data transfers to and from LPDDR4 memory.

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| D1 | `covariance_estimation_0` | `m_axi_gmem` | `axi_noc_0` | `S01_AXI` | AXI4 |
| D2 | `mvdr_weights_0` | `m_axi_gmem0` | `axi_noc_0` | `S02_AXI` | AXI4 |
| D3 | `mvdr_weights_0` | `m_axi_gmem1` | `smartconnect_1` | `S00_AXI` | AXI4 |
| D4 | `mvdr_weights_0` | `m_axi_gmem2` | `smartconnect_1` | `S01_AXI` | AXI4 |
| D5 | `smartconnect_1` | `M00_AXI` | `axi_noc_0` | `S03_AXI` | AXI4 |

> **gmem port names:** In your HLS code, `bundle=gmem` → port name `m_axi_gmem`. `bundle=gmem0` → `m_axi_gmem0`. etc. The port names on the block exactly match the bundle names in your `#pragma HLS INTERFACE m_axi` lines.

---

## STEP 24 — PS to NoC Connection (PS DDR access)

The PS needs to write input data to DDR and read weight results from DDR.

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| N1 | `versal_cips_0` | `NOC_PS_CCI_0` | `axi_noc_0` | `S00_AXI` | AXI4 |

> **Port name note:** The exact name of the CIPS NoC port depends on which NoC interface you enabled in Step 13b. It may appear as `NOC_PS_CCI_0`, `NOC_LPD_AXI_0`, or `PS_NOC_LPD_AXI_0`. Look at the left side of the `versal_cips_0` block for a port with `NOC` in its name and connect it to `S00_AXI` on the NoC.

---

## STEP 25 — LPDDR4 Reference Clock Connections

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| K1 | `util_ds_buf_0` | `IBUF_OUT` | `axi_noc_0` | `sys_clk0` |

---

## STEP 26 — Make External Ports

Some ports need to be exposed as board-level I/O pins.

### 26a — LPDDR4 clock differential pair

1. Right-click the `util_ds_buf_0` port named **`IBUF_DS_P`**.
2. Click **Make External** (or press **Ctrl+T**).
3. A new port appears at the edge of the canvas labeled `IBUF_DS_P`.
4. Click on that port label to select it. Press **F2** to rename it.
5. Type: `lpddr4_clk_p` and press **Enter**.

6. Right-click the `util_ds_buf_0` port named **`IBUF_DS_N`**.
7. Click **Make External**.
8. Rename it to `lpddr4_clk_n`.

### 26b — LPDDR4 DDR physical interface

1. On the `axi_noc_0` block, find the port named **`CH0_LPDDR4_0`** (it may also appear as `CH0_DDR4_0` depending on your Vivado version — it is the physical memory interface port going to the board's LPDDR4 chip).
2. Right-click it → **Make External**.
3. Leave the name as is — Vivado will assign the correct board-level pin connections through the board preset.

### 26c — AXI-Stream input for RDBS

The `rdbs_kernel_0 : x_in` port receives raw ADC data. Make it external for now.

1. Right-click the `rdbs_kernel_0` port named **`x_in`**.
2. Click **Make External**.
3. The port appears at the canvas edge named `x_in`.

> **Note:** In your final system, `x_in` will connect to an AXI DMA IP that fetches raw ADC data from DDR. For this integration step, making it external lets you test the design without needing a DMA. You can add the DMA in a later revision.

---

Press **Ctrl+S** to save.

---

---

# PART 6 — RUN CONNECTION AUTOMATION

After making manual connections, Vivado may offer automatic assistance for clock/reset wiring you missed.

## STEP 27 — Use Run Connection Automation

1. Look at the top of the canvas. You should see a **green banner** that says:
   **"Designer Assistance available. Run Connection Automation"**
2. Click **Run Connection Automation**.
3. A dialog opens listing all ports that can be auto-connected.
4. On the left tree, tick **All Automation** (the top checkbox).
5. Review what Vivado plans to connect. This usually covers:
   - Remaining clock pins on IPs you may have missed.
   - Board-component connections like `sys_clk0` to the LPDDR4 sys clock.
6. Click **OK**.
7. Vivado makes the connections. Review the canvas to confirm nothing unexpected was changed.

> **Important:** After running automation, manually verify that `s_axi_ctrl` on all three HLS IPs is still connected to `smartconnect_0` masters. Sometimes automation re-routes these. If it changed them, undo (Ctrl+Z) and redo that connection manually.

---

---

# PART 7 — ADDRESS EDITOR

## STEP 28 — Open Address Editor

1. At the top of the block design window, click the **Address Editor** tab (it is next to the **Diagram** tab).
2. The Address Editor opens showing a tree of all master interfaces.

---

## STEP 29 — Assign AXI-Lite Control Register Addresses

Expand the tree under the PS master (look for `versal_cips_0` or `M_AXI_FPD`).

You need to assign addresses to the `s_axi_ctrl` of each HLS IP.

**Option A — Auto Assign (easiest):**
1. Click the **Assign All** button in the Address Editor toolbar (it looks like a lightning bolt or is labeled "Assign All").
2. Vivado picks non-conflicting addresses automatically.
3. Review the assigned addresses and note them down — you will need them in your PS software.

**Option B — Manual assignment (if you want specific addresses):**
1. Find `rdbs_kernel_0 / s_axi_ctrl` in the Address Editor.
2. Right-click its **Master Base Address** cell → **Assign Address**.
   - Or double-click the cell and type: `0xA0000000`
   - Range: 4K
3. Find `covariance_estimation_0 / s_axi_ctrl`.
   - Assign address: `0xA0001000`
   - Range: 4K
4. Find `mvdr_weights_0 / s_axi_ctrl`.
   - Assign address: `0xA0002000`
   - Range: 4K

---

## STEP 30 — Assign AXI4 Master DDR Addresses

Expand the tree under each HLS IP's AXI4 master ports.

1. Find `covariance_estimation_0 / m_axi_gmem`.
   - Right-click the **Slave Segment** cell.
   - Select **Assign**. Vivado maps it to DDR LOW0 (starts at 0x00000000, size 2GB).
2. Find `mvdr_weights_0 / m_axi_gmem0`.
   - Assign to DDR LOW0.
3. Find `mvdr_weights_0 / m_axi_gmem1`.
   - Assign to DDR LOW0.
4. Find `mvdr_weights_0 / m_axi_gmem2`.
   - Assign to DDR LOW0.

> All four can be mapped to the same DDR LOW0 segment. The specific buffer locations within DDR (where R is stored, where a is stored, where w is stored) are set at runtime by the PS software writing addresses into the AXI-Lite control registers. The mapping here just means "these masters are allowed to access DDR."

Click **Assign All** at the top to auto-assign any remaining unmapped entries.

---

Press **Ctrl+S** to save.

---

---

# PART 8 — VALIDATE THE DESIGN

## STEP 31 — Run Design Validation

1. Click back on the **Diagram** tab to see the canvas.
2. Press **F6** on your keyboard, or right-click the canvas → **Validate Design**.
3. Vivado runs all Design Rule Checks (DRCs). Wait for it to finish.

### Expected result:
A popup says: **"Validation successful. There are no errors or critical warnings in this design."**

Click **OK**.

### If you get errors, fix them using this table:

| Error message | What it means | How to fix it |
|---|---|---|
| `Clock domain crossing between X and Y` | Two connected IPs are on different clocks | Verify all `ap_clk` ports connect to the same `clk_wiz_0 : clk_out1` wire |
| `S0X_AXI of axi_noc_0 has no associated clock` | NoC slave clock not set | Double-click `axi_noc_0` → Inputs tab → set Associated Clock for that port to `aclk0` |
| `Master port has no path to any slave` | Not all NoC connectivity boxes ticked | Double-click `axi_noc_0` → Connectivity tab → tick missing box |
| `proc_sys_reset_0/dcm_locked is not connected` | Missing locked connection | Connect `clk_wiz_0 : locked` → `proc_sys_reset_0 : dcm_locked` |
| `Address not assigned for port X` | Address Editor entry missing | Open Address Editor → click Assign All |
| `AXI protocol mismatch on port X` | Wrong protocol type connection | AXI4 masters must go to NoC slaves. AXI-Lite must go to SmartConnect only |
| `No driver for port ap_rst_n` | Reset not connected to an IP | Connect `proc_sys_reset_0 : peripheral_aresetn[0:0]` to the IP's `ap_rst_n` |
| `s_axi_ctrl has no master` | AXI-Lite control not connected | Connect `smartconnect_0 : M0X_AXI` to the IP's `s_axi_ctrl` port |

---

---

# PART 9 — GENERATE HDL WRAPPER

## STEP 32 — Create the HDL Wrapper

1. Click the **Sources** tab (left panel, usually below the Flow Navigator).
2. In the Sources panel, expand **Design Sources**.
3. Find `mvdr_bd.bd` in the list.
4. Right-click on `mvdr_bd.bd`.
5. Click **Create HDL Wrapper…**
6. A dialog appears with two options:
   - Select **"Let Vivado manage wrapper and auto-update"**.
7. Click **OK**.
8. Vivado generates `mvdr_bd_wrapper.v`.
9. In the Sources panel, you will see `mvdr_bd_wrapper` appear at the top level under Design Sources. This is your design top.

---

---

# PART 10 — ADD XDC CONSTRAINTS

## STEP 33 — Add a Constraints File

1. In the Flow Navigator, click **Add Sources** (under Project Manager, or use **File → Add Sources**).
2. Select **Add or Create Constraints** → click **Next**.
3. Click **Create File**.
4. Set:
   - **File type**: XDC
   - **File name**: `vck190_mvdr`
5. Click **OK**, then **Finish**.
6. In the Sources panel, under **Constraints**, double-click `vck190_mvdr.xdc` to open it in the editor.

## STEP 34 — Add Constraints Content

Paste all of this into the XDC file and save (Ctrl+S):

```tcl
###############################################################################
# VCK190 LPDDR4 Reference Clock — 100 MHz differential input
# These are the board-level pins for the LPDDR4 sys_clk input.
# Verify exact pin numbers against your VCK190 board revision XDC at:
# <Vivado_install>/data/boards/board_files/vck190/<rev>/vck190.xdc
###############################################################################
set_property PACKAGE_PIN BH51 [get_ports {lpddr4_clk_p}]
set_property PACKAGE_PIN BJ51 [get_ports {lpddr4_clk_n}]
set_property IOSTANDARD LVDS  [get_ports {lpddr4_clk_p}]
set_property IOSTANDARD LVDS  [get_ports {lpddr4_clk_n}]

###############################################################################
# False path on PL reset — crosses to PL clock domain, but it is
# asynchronous by design and synchronised by proc_sys_reset_0
###############################################################################
set_false_path -from [get_ports pl0_resetn]

###############################################################################
# PL clock timing — 300 MHz = 3.333 ns period
# Vivado auto-creates this from the Clocking Wizard but add it
# explicitly if synthesis reports an unconstrained clock.
###############################################################################
# Uncomment the line below ONLY if synthesis warns about unconstrained clock:
# create_clock -period 3.333 -name pl_clk300 [get_pins clk_wiz_0/inst/mmcme4_adv_inst/CLKOUT0]
```

> **PIN VERIFICATION:** After saving, check `<Vivado_install>/data/boards/board_files/vck190/` for the official VCK190 XDC. Open it and search for `LPDDR` or `sys_clk` to find the exact pin assignments for your board revision. Update the `PACKAGE_PIN` values if they differ.

---

---

# PART 11 — SYNTHESIS, IMPLEMENTATION, BITSTREAM

## STEP 35 — Run Synthesis

1. In the Flow Navigator, under **SYNTHESIS**, click **Run Synthesis**.
2. In the **Launch Runs** dialog:
   - Leave **Launch directory** as default.
   - **Number of jobs**: set to the number of CPU cores on your machine (e.g. 8).
3. Click **OK**.
4. Vivado starts synthesis. A progress bar appears in the top-right corner of the window.
5. **Expected time**: 15–30 minutes.
6. When synthesis completes, a dialog appears:
   - Click **Open Synthesized Design** if you want to review utilization.
   - Or click **Run Implementation** directly.

**After synthesis, check these reports:**
- Click **Reports → Report Utilization** to verify your three HLS IPs use a small percentage of resources (expect <5% of VCK190's LUTs, FFs, and DSPs).

---

## STEP 36 — Run Implementation

1. In the Flow Navigator, under **IMPLEMENTATION**, click **Run Implementation**.
2. In the **Launch Runs** dialog, click **OK**.
3. **Expected time**: 30–90 minutes (Versal implementation is slower than UltraScale due to NoC compilation).
4. When implementation completes, a dialog appears. Click **Open Implemented Design**.
5. Check timing by clicking **Reports → Timing → Report Timing Summary**.

**Timing check:**
- Look at the **WNS (Worst Negative Slack)** column for your `pl_clk300` clock.
- If WNS ≥ 0.000 ns → timing is **met** ✓
- If WNS < 0.000 ns → timing is **violated**. Do this to fix it:
  1. Double-click `clk_wiz_0` in the block design.
  2. Change `clk_out1` from 300 MHz to **250 MHz**.
  3. Save, re-run synthesis and implementation.

---

## STEP 37 — Generate Device Image (Bitstream)

> In Vivado 2025.2 for Versal devices, the output is a **PDI (Programmable Device Image)**, not a traditional bitstream. The button in the Flow Navigator may say **Generate Bitstream** or **Generate Device Image** — both produce the same PDI file.

1. In the Flow Navigator, under **PROGRAM AND DEBUG**, click **Generate Bitstream** (or **Generate Device Image**).
2. If prompted "Implementation not up to date", click **Yes** to re-run.
3. In the **Launch Runs** dialog, click **OK**.
4. **Expected time**: 10–20 minutes.
5. When complete, a dialog appears. Click **Cancel** (no need to open the implemented design again).

---

## STEP 38 — Export Hardware Platform (XSA)

1. In the top menu, click **File → Export → Export Hardware**.
2. In the Export Hardware dialog:
   - **Output**: select **Include bitstream** (or **Pre-synthesis** if bitstream isn't ready).
   - Click **Next**.
3. **XSA file name**: type `mvdr_system`
4. **Export to**: click **…** and choose your project directory (e.g. `D:/vivado_projects/mvdr_system/`).
5. Click **Next**.
6. Review the summary. Click **Finish**.
7. The file `mvdr_system.xsa` is created at the location you chose.

> This XSA file contains the full hardware description including the PDI and the IP address map. You import it into **Vitis** (or PetaLinux) to write the PS-side software.

---

---

# PART 12 — COMPLETE CONNECTION REFERENCE TABLE

Every single connection in the design in one place. Use this as your checklist.

## Clock and Reset Connections

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| C1 | versal_cips_0 | pl0_ref_clk | clk_wiz_0 | clk_in1 |
| C2 | clk_wiz_0 | clk_out1 | proc_sys_reset_0 | slowest_sync_clk |
| C3 | clk_wiz_0 | clk_out1 | smartconnect_0 | aclk |
| C4 | clk_wiz_0 | clk_out1 | smartconnect_1 | aclk |
| C5 | clk_wiz_0 | clk_out1 | rdbs_kernel_0 | ap_clk |
| C6 | clk_wiz_0 | clk_out1 | covariance_estimation_0 | ap_clk |
| C7 | clk_wiz_0 | clk_out1 | mvdr_weights_0 | ap_clk |
| C8 | clk_wiz_0 | clk_out1 | axi_noc_0 | aclk0 |
| C9 | clk_wiz_0 | clk_out1 | axi_noc_0 | aclk1 |
| C10 | clk_wiz_0 | locked | proc_sys_reset_0 | dcm_locked |
| R1 | versal_cips_0 | pl0_resetn | clk_wiz_0 | resetn |
| R2 | versal_cips_0 | pl0_resetn | proc_sys_reset_0 | ext_reset_in |
| R3 | proc_sys_reset_0 | peripheral_aresetn[0:0] | rdbs_kernel_0 | ap_rst_n |
| R4 | proc_sys_reset_0 | peripheral_aresetn[0:0] | covariance_estimation_0 | ap_rst_n |
| R5 | proc_sys_reset_0 | peripheral_aresetn[0:0] | mvdr_weights_0 | ap_rst_n |
| R6 | proc_sys_reset_0 | peripheral_aresetn[0:0] | smartconnect_0 | aresetn |
| R7 | proc_sys_reset_0 | peripheral_aresetn[0:0] | smartconnect_1 | aresetn |

## Data and Control Connections

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| S1 | rdbs_kernel_0 | y_out | covariance_estimation_0 | y_in | AXI4-Stream |
| L1 | versal_cips_0 | M_AXI_FPD | smartconnect_0 | S00_AXI | AXI4-Lite |
| L2 | smartconnect_0 | M00_AXI | rdbs_kernel_0 | s_axi_ctrl | AXI4-Lite |
| L3 | smartconnect_0 | M01_AXI | covariance_estimation_0 | s_axi_ctrl | AXI4-Lite |
| L4 | smartconnect_0 | M02_AXI | mvdr_weights_0 | s_axi_ctrl | AXI4-Lite |
| D1 | covariance_estimation_0 | m_axi_gmem | axi_noc_0 | S01_AXI | AXI4 |
| D2 | mvdr_weights_0 | m_axi_gmem0 | axi_noc_0 | S02_AXI | AXI4 |
| D3 | mvdr_weights_0 | m_axi_gmem1 | smartconnect_1 | S00_AXI | AXI4 |
| D4 | mvdr_weights_0 | m_axi_gmem2 | smartconnect_1 | S01_AXI | AXI4 |
| D5 | smartconnect_1 | M00_AXI | axi_noc_0 | S03_AXI | AXI4 |
| N1 | versal_cips_0 | NOC_PS_CCI_0 | axi_noc_0 | S00_AXI | AXI4 |
| K1 | util_ds_buf_0 | IBUF_OUT | axi_noc_0 | sys_clk0 | (clock) |

## External Ports

| Port Name | Direction | Connected To |
|---|---|---|
| lpddr4_clk_p | Input | util_ds_buf_0 : IBUF_DS_P |
| lpddr4_clk_n | Input | util_ds_buf_0 : IBUF_DS_N |
| CH0_LPDDR4_0 (or CH0_DDR4_0) | Inout | axi_noc_0 : CH0_LPDDR4_0 |
| x_in | Input (AXI-S) | rdbs_kernel_0 : x_in |

---

## Final Checklist Before Synthesis

- [ ] STEP 3: All 3 HLS IPs visible in IP Catalog (rdbs, covariance, mvdr)
- [ ] STEP 13: CIPS configured — Full System, M_AXI_FPD enabled, pl0_ref_clk at 200 MHz, 1 PL reset
- [ ] STEP 14: AXI NoC — 4 slaves, 1 master, 2 clocks, LPDDR4 SDRAM, 625ps memory clock
- [ ] STEP 14: NoC Connectivity tab — all 4 S→M00 checkboxes ticked
- [ ] STEP 14: NoC QoS tab — all 4 ports set to BEST_EFFORT
- [ ] STEP 15: Clocking Wizard — 200 MHz in, 300 MHz out, active-low reset, locked output enabled
- [ ] STEP 16: smartconnect_0 — 1 slave, 3 masters
- [ ] STEP 17: smartconnect_1 — 2 slaves, 1 master
- [ ] STEP 18: util_ds_buf_0 — type set to IBUFDS
- [ ] STEP 19: All 10 clock connections made (C1–C10)
- [ ] STEP 20: All 7 reset connections made (R1–R7)
- [ ] STEP 21: AXI-Stream connection S1 made (rdbs y_out → covariance y_in)
- [ ] STEP 22: All 4 AXI-Lite connections made (L1–L4)
- [ ] STEP 23: All 5 AXI4 DDR connections made (D1–D5)
- [ ] STEP 24: PS→NoC connection N1 made
- [ ] STEP 25: LPDDR4 clock connection K1 made
- [ ] STEP 26: External ports created (lpddr4_clk_p/n, CH0_LPDDR4_0, x_in)
- [ ] STEP 28–30: Address Editor — all AXI-Lite and AXI4 addresses assigned
- [ ] STEP 31: Design validated — ZERO errors
- [ ] STEP 32: HDL wrapper created (mvdr_bd_wrapper.v)
- [ ] STEP 34: XDC file added and LPDDR4 pin constraints entered

---

## PS Software Runtime Flow

Once your XSA is imported into Vitis and you have a bare-metal application running on the APU (A72):

```
1. Allocate three DDR buffers using XMemAlloc or static addresses:
      R_buf   = 0x10000000   (21×21 complex entries × 8 bytes = 3528 bytes)
      a_buf   = 0x10002000   (21 complex entries × 8 bytes = 168 bytes)
      w_buf   = 0x10003000   (21 complex entries × 8 bytes = 168 bytes)

2. Load your input data into DDR:
      Write raw ADC data to the DMA (or directly stream it to rdbs_kernel_0 x_in)
      Write steering vector a to a_buf

3. Set covariance kernel DDR output address:
      Write R_buf address → covariance_estimation_0 AXI-Lite register offset 0x10
      Write R_buf address upper 32 bits → offset 0x14 (usually 0x00000000)

4. Start both RDBS and covariance:
      Write 1000 (K_SNAPSHOTS) → rdbs_kernel_0 s_axi_ctrl + register offset for num_snapshots
      Write 0x01 → rdbs_kernel_0 s_axi_ctrl + 0x00  (ap_start = 1)
      Write 0x01 → covariance_estimation_0 s_axi_ctrl + 0x00  (ap_start = 1)
      (covariance waits on AXI-Stream; rdbs starts producing data)

5. Poll covariance done:
      Loop: read covariance_estimation_0 s_axi_ctrl + 0x00
            check bit[1] == 1  (ap_done)

6. Set MVDR input/output addresses:
      Write R_buf → mvdr_weights_0 s_axi_ctrl + offset for R_in
      Write a_buf → mvdr_weights_0 s_axi_ctrl + offset for a_in
      Write w_buf → mvdr_weights_0 s_axi_ctrl + offset for w_out
      Write 0x01 → mvdr_weights_0 s_axi_ctrl + 0x00  (ap_start = 1)

7. Poll MVDR done:
      Loop: read mvdr_weights_0 s_axi_ctrl + 0x00
            check bit[1] == 1  (ap_done)

8. Read weights from DDR:
      Read 21 complex ap_fixed<32,9> pairs from w_buf
      These are your MVDR beamforming weights.

NOTE: Exact register offsets are in the Vitis HLS synthesis report.
      In your HLS project → solution1 → syn → report → [kernel_name]_csynth.rpt
      Search for "AXI-Lite slave register map" in that file.
```

