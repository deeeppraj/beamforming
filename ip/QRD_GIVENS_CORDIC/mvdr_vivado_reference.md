# MVDR System — Vivado 2025.2 Reference (VCK190)
## IP Customization + Complete Connection Tables

---

## 1. IP Customization Settings

### versal_cips_0 (Control, Interfaces & Processing System)

| Setting | Value |
|---|---|
| Design Flow | Full System |
| M_AXI_FPD | Enabled |
| Number of PL Resets | 1 |
| NOC Interface | FPD_CCI_NOC_0/1/2/3 (all 4 enabled) |
| PL0_REF_CLK | Enabled, 200 MHz |

---

### axi_noc_0 (AXI NoC)

#### General Tab

| Setting | Value |
|---|---|
| Number of AXI Slave Interfaces | 7 |
| Number of AXI Master Interfaces | 1 |
| Number of AXI Clocks | 5 |
| Number of Inter-NoC Slave Interfaces | 0 |
| Number of Inter-NoC Master Interfaces | 0 |
| Memory Controller | Single Memory Controller |
| Number of Memory Controller Ports | 1 |
| DDR Address Region 0 | DDR LOW0 (0x0000 0000 0000 to 0x0000 7FFF FFFF; 2G) |
| DDR Address Region 1 | NONE |

#### Inputs Tab — Connected To

| Port | Connected To |
|---|---|
| S00_AXI | PS Cache Coherent |
| S01_AXI | PS Cache Coherent |
| S02_AXI | PS Cache Coherent |
| S03_AXI | PS Cache Coherent |
| S04_AXI | PL |
| S05_AXI | PL |
| S06_AXI | PL |

#### Associated Clocks (set in General Tab slave table)

| Port | Associated Clock |
|---|---|
| S00_AXI | aclk1 |
| S01_AXI | aclk2 |
| S02_AXI | aclk3 |
| S03_AXI | aclk4 |
| S04_AXI | aclk0 |
| S05_AXI | aclk0 |
| S06_AXI | aclk0 |
| M00_AXI | aclk0 |

#### Connectivity Tab

| Slave | Master | Tick |
|---|---|---|
| S00_AXI | M00_AXI | ✓ |
| S01_AXI | M00_AXI | ✓ |
| S02_AXI | M00_AXI | ✓ |
| S03_AXI | M00_AXI | ✓ |
| S04_AXI | M00_AXI | ✓ |
| S05_AXI | M00_AXI | ✓ |
| S06_AXI | M00_AXI | ✓ |

#### QoS Tab

| Port | Traffic Class |
|---|---|
| S00_AXI | BEST_EFFORT |
| S01_AXI | BEST_EFFORT |
| S02_AXI | BEST_EFFORT |
| S03_AXI | BEST_EFFORT |
| S04_AXI | BEST_EFFORT |
| S05_AXI | BEST_EFFORT |
| S06_AXI | BEST_EFFORT |

#### DDR Basic Tab

| Setting | Value |
|---|---|
| Controller Type | LPDDR4 SDRAM |
| Clock Selection | Memory Clock |
| Memory Clock Period (ps) | 625 |
| Input System Clock Period (ps) | 10000 |
| System Clock | Differential |
| Enable Internal Responder | Unchecked |

#### DDR Memory Tab

| Setting | Value |
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
| ECC | Unchecked |
| Write DBI | DM NO DBI |
| Read DBI | Unchecked |

---

### clk_wiz_0 (Clocking Wizard)

| Setting | Value |
|---|---|
| Input Frequency | 199.998001 MHz (matches CIPS actual output) |
| Output clk_out1 Frequency | 300 MHz |
| Reset Type | Active Low |
| locked output | Enabled |

---

### smartconnect_0 (AXI-Lite fan-out)

| Setting | Value |
|---|---|
| Number of Slave Interfaces | 1 |
| Number of Master Interfaces | 3 |

---

### smartconnect_1 (MVDR DDR merger)

| Setting | Value |
|---|---|
| Number of Slave Interfaces | 2 |
| Number of Master Interfaces | 1 |

---

### proc_sys_reset_0

No changes — all defaults.

---

### util_ds_buf_0 (Utility Buffer)

| Setting | Value |
|---|---|
| C Buf Type | IBUFDS |

---

---

## 2. Complete Connection Tables

### Clock Connections

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
| C9 | clk_wiz_0 | locked | proc_sys_reset_0 | dcm_locked |
| C10 | versal_cips_0 | fpd_cci_noc_axi0_clk | axi_noc_0 | aclk1 |
| C11 | versal_cips_0 | fpd_cci_noc_axi1_clk | axi_noc_0 | aclk2 |
| C12 | versal_cips_0 | fpd_cci_noc_axi2_clk | axi_noc_0 | aclk3 |
| C13 | versal_cips_0 | fpd_cci_noc_axi3_clk | axi_noc_0 | aclk4 |

---

### Reset Connections

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| R1 | versal_cips_0 | pl0_resetn | clk_wiz_0 | resetn |
| R2 | versal_cips_0 | pl0_resetn | proc_sys_reset_0 | ext_reset_in |
| R3 | proc_sys_reset_0 | peripheral_aresetn[0:0] | rdbs_kernel_0 | ap_rst_n |
| R4 | proc_sys_reset_0 | peripheral_aresetn[0:0] | covariance_estimation_0 | ap_rst_n |
| R5 | proc_sys_reset_0 | peripheral_aresetn[0:0] | mvdr_weights_0 | ap_rst_n |
| R6 | proc_sys_reset_0 | peripheral_aresetn[0:0] | smartconnect_0 | aresetn |
| R7 | proc_sys_reset_0 | peripheral_aresetn[0:0] | smartconnect_1 | aresetn |

---

### AXI-Stream Connection

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| S1 | rdbs_kernel_0 | y_out | covariance_estimation_0 | y_in | AXI4-Stream |

---

### AXI-Lite Control Connections

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| L1 | versal_cips_0 | M_AXI_FPD | smartconnect_0 | S00_AXI | AXI4-Lite |
| L2 | smartconnect_0 | M00_AXI | rdbs_kernel_0 | s_axi_ctrl | AXI4-Lite |
| L3 | smartconnect_0 | M01_AXI | covariance_estimation_0 | s_axi_ctrl | AXI4-Lite |
| L4 | smartconnect_0 | M02_AXI | mvdr_weights_0 | s_axi_ctrl | AXI4-Lite |

---

### AXI4 DDR Connections (PL Masters → NoC)

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| D1 | covariance_estimation_0 | m_axi_gmem | axi_noc_0 | S04_AXI | AXI4 |
| D2 | mvdr_weights_0 | m_axi_gmem0 | axi_noc_0 | S05_AXI | AXI4 |
| D3 | mvdr_weights_0 | m_axi_gmem1 | smartconnect_1 | S00_AXI | AXI4 |
| D4 | mvdr_weights_0 | m_axi_gmem2 | smartconnect_1 | S01_AXI | AXI4 |
| D5 | smartconnect_1 | M00_AXI | axi_noc_0 | S06_AXI | AXI4 |

---

### PS CCI → NoC Connections

| # | From Block | From Port | To Block | To Port | Protocol |
|---|---|---|---|---|---|
| N1 | versal_cips_0 | FPD_CCI_NOC_0 | axi_noc_0 | S00_AXI | AXI4 |
| N2 | versal_cips_0 | FPD_CCI_NOC_1 | axi_noc_0 | S01_AXI | AXI4 |
| N3 | versal_cips_0 | FPD_CCI_NOC_2 | axi_noc_0 | S02_AXI | AXI4 |
| N4 | versal_cips_0 | FPD_CCI_NOC_3 | axi_noc_0 | S03_AXI | AXI4 |

---

### LPDDR4 Clock Connection

| # | From Block | From Port | To Block | To Port |
|---|---|---|---|---|
| K1 | util_ds_buf_0 | IBUF_OUT | axi_noc_0 | sys_clk0_clk_p[0:0] |

---

### External Ports

| Port Name | Direction | Connected To |
|---|---|---|
| lpddr4_clk_p | Input | util_ds_buf_0 : IBUF_DS_P |
| lpddr4_clk_n | Input | util_ds_buf_0 : IBUF_DS_N |
| CH0_LPDDR4_0 | Inout | axi_noc_0 : CH0_LPDDR4_0 |
| x_in_0 | Input (AXI-S) | rdbs_kernel_0 : x_in |

---

## 3. Address Map (Auto-assigned by Vivado)

| Master | Slave IP | Port | Base Address | Range |
|---|---|---|---|---|
| versal_cips_0 / M_AXI_FPD | covariance_estimation_0 | s_axi_ctrl | 0xA400_0000 | 64K |
| versal_cips_0 / M_AXI_FPD | mvdr_weights_0 | s_axi_ctrl | 0xA401_0000 | 64K |
| versal_cips_0 / M_AXI_FPD | rdbs_kernel_0 | s_axi_ctrl | 0xA402_0000 | 64K |
| All DDR masters | axi_noc_0 | M00_AXI → LPDDR4 | 0x0000_0000_0000 | 2G |
