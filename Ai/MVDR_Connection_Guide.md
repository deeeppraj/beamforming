# MVDR System — Vivado 2025.2 Connection Guide (VCK190)
## Complete Wiring Reference — Verified Working Build

> This is the connection-only reference for the working MVDR system block design.
> IP customization settings are documented separately in `mvdr_vivado_reference.md`
> and are not repeated here.

---

## Block Inventory

| Instance Name | IP |
|---|---|
| `versal_cips_0` | Control, Interfaces and Processing System |
| `axi_noc_0` | AXI NoC |
| `clk_wiz_0` | Clocking Wizard |
| `proc_sys_reset_0` | Processor System Reset |
| `smartconnect_0` | AXI SmartConnect (AXI-Lite fan-out) |
| `smartconnect_1` | AXI SmartConnect (MVDR DDR merger) |
| `util_ds_buf_0` | Utility Buffer (IBUFDS) |
| `rdbs_kernel_0` | RDBS HLS IP |
| `covariance_estimation_0` | Covariance HLS IP |
| `mvdr_weights_0` | MVDR HLS IP |

---

## 1. Clock Connections

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
| C9 | `clk_wiz_0` | `locked` | `proc_sys_reset_0` | `dcm_locked` |
| C10 | `versal_cips_0` | `fpd_cci_noc_axi0_clk` | `axi_noc_0` | `aclk1` |
| C11 | `versal_cips_0` | `fpd_cci_noc_axi1_clk` | `axi_noc_0` | `aclk2` |
| C12 | `versal_cips_0` | `fpd_cci_noc_axi2_clk` | `axi_noc_0` | `aclk3` |
| C13 | `versal_cips_0` | `fpd_cci_noc_axi3_clk` | `axi_noc_0` | `aclk4` |

> **C3–C8 all source from the same wire** (`clk_wiz_0 : clk_out1`). Make C2 first, then right-click the wire → **Add Junction** for each subsequent destination.
> **C10–C13 are 4 distinct clocks**, one per PS coherent port — these are NOT the same wire, each is its own `fpd_cci_noc_axi{n}_clk` output from CIPS.

---

## 2. Reset Connections

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| R1 | `versal_cips_0` | `pl0_resetn` | `clk_wiz_0` | `resetn` |
| R2 | `versal_cips_0` | `pl0_resetn` | `proc_sys_reset_0` | `ext_reset_in` |
| R3 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `rdbs_kernel_0` | `ap_rst_n` |
| R4 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `covariance_estimation_0` | `ap_rst_n` |
| R5 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `mvdr_weights_0` | `ap_rst_n` |
| R6 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `smartconnect_0` | `aresetn` |
| R7 | `proc_sys_reset_0` | `peripheral_aresetn[0:0]` | `smartconnect_1` | `aresetn` |

> **R1 and R2 share a source** (`pl0_resetn`) — add a junction for the second connection.
> **R3–R7 share a source** (`peripheral_aresetn[0:0]`) — add junctions for each.

---

## 3. AXI-Stream Connection (RDBS → Covariance)

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| S1 | `rdbs_kernel_0` | `y_out` | `covariance_estimation_0` | `y_in` | AXI4-Stream |

---

## 4. AXI-Lite Control Connections

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| L1 | `versal_cips_0` | `M_AXI_FPD` | `smartconnect_0` | `S00_AXI` | AXI4-Lite |
| L2 | `smartconnect_0` | `M00_AXI` | `rdbs_kernel_0` | `s_axi_ctrl` | AXI4-Lite |
| L3 | `smartconnect_0` | `M01_AXI` | `covariance_estimation_0` | `s_axi_ctrl` | AXI4-Lite |
| L4 | `smartconnect_0` | `M02_AXI` | `mvdr_weights_0` | `s_axi_ctrl` | AXI4-Lite |

---

## 5. AXI4 DDR Connections (PL Masters → NoC)

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| D1 | `covariance_estimation_0` | `m_axi_gmem` | `axi_noc_0` | `S04_AXI` | AXI4 |
| D2 | `mvdr_weights_0` | `m_axi_gmem0` | `axi_noc_0` | `S05_AXI` | AXI4 |
| D3 | `mvdr_weights_0` | `m_axi_gmem1` | `smartconnect_1` | `S00_AXI` | AXI4 |
| D4 | `mvdr_weights_0` | `m_axi_gmem2` | `smartconnect_1` | `S01_AXI` | AXI4 |
| D5 | `smartconnect_1` | `M00_AXI` | `axi_noc_0` | `S06_AXI` | AXI4 |

---

## 6. PS Cache-Coherent → NoC Connections

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| N1 | `versal_cips_0` | `FPD_CCI_NOC_0` | `axi_noc_0` | `S00_AXI` | AXI4 |
| N2 | `versal_cips_0` | `FPD_CCI_NOC_1` | `axi_noc_0` | `S01_AXI` | AXI4 |
| N3 | `versal_cips_0` | `FPD_CCI_NOC_2` | `axi_noc_0` | `S02_AXI` | AXI4 |
| N4 | `versal_cips_0` | `FPD_CCI_NOC_3` | `axi_noc_0` | `S03_AXI` | AXI4 |

---

## 7. LPDDR4 Reference Clock Connection

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| K1 | `util_ds_buf_0` | `IBUF_OUT` | `axi_noc_0` | `sys_clk0_clk_p[0:0]` |

---

## 8. External Ports

| Port Name | Direction | Connected To |
|---|---|---|
| `lpddr4_clk_p` | Input | `util_ds_buf_0 : IBUF_DS_P` |
| `lpddr4_clk_n` | Input | `util_ds_buf_0 : IBUF_DS_N` |
| `CH0_LPDDR4_0` | Inout | `axi_noc_0 : CH0_LPDDR4_0` |
| `x_in_0` | Input (AXI-Stream) | `rdbs_kernel_0 : x_in` |

---

## 9. Address Map

| Master | Slave IP | Port | Base Address | Range |
|---|---|---|---|---|
| `versal_cips_0 / M_AXI_FPD` | `covariance_estimation_0` | `s_axi_ctrl` | `0xA400_0000` | 64K |
| `versal_cips_0 / M_AXI_FPD` | `mvdr_weights_0` | `s_axi_ctrl` | `0xA401_0000` | 64K |
| `versal_cips_0 / M_AXI_FPD` | `rdbs_kernel_0` | `s_axi_ctrl` | `0xA402_0000` | 64K |
| All DDR masters (`m_axi_gmem`, `m_axi_gmem0/1/2`) | `axi_noc_0` | `M00_AXI` → LPDDR4 | `0x0000_0000_0000` | 2G |

---

## Connection Count Summary

| Category | Count |
|---|---|
| Clock connections | 13 (C1–C13) |
| Reset connections | 7 (R1–R7) |
| AXI-Stream connections | 1 (S1) |
| AXI-Lite control connections | 4 (L1–L4) |
| AXI4 DDR connections (PL→NoC) | 5 (D1–D5) |
| PS coherent→NoC connections | 4 (N1–N4) |
| LPDDR4 clock connections | 1 (K1) |
| External ports | 4 |
| **Total wires** | **35** |

---

## Build-Order Checklist

- [ ] All 13 clock connections made (C1–C13)
- [ ] All 7 reset connections made (R1–R7)
- [ ] AXI-Stream S1 made (rdbs → covariance)
- [ ] All 4 AXI-Lite connections made (L1–L4)
- [ ] All 5 AXI4 DDR connections made (D1–D5)
- [ ] All 4 PS coherent NoC connections made (N1–N4)
- [ ] LPDDR4 clock connection made (K1)
- [ ] All 4 external ports created and named correctly
- [ ] Address Editor: all 4 entries assigned per the table above
- [ ] Design validated (F6) — zero errors
