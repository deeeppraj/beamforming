//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2025.2 (win64) Build 6299465 Fri Nov 14 19:35:11 GMT 2025
//Date        : Sat Jun 27 23:08:53 2026
//Host        : Deepraj running 64-bit major release  (build 9200)
//Command     : generate_target mvdrr.bd
//Design      : mvdrr
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

(* CORE_GENERATION_INFO = "mvdrr,IP_Integrator,{x_ipVendor=xilinx.com,x_ipLibrary=BlockDiagram,x_ipName=mvdrr,x_ipVersion=1.00.a,x_ipLanguage=VERILOG,numBlks=10,numReposBlks=10,numNonXlnxBlks=0,numHierBlks=0,maxHierDepth=0,numSysgenBlks=0,numHlsBlks=3,numHdlrefBlks=0,numPkgbdBlks=0,bdsource=USER,da_board_cnt=2,da_cips_cnt=1,synth_mode=Hierarchical}" *) (* HW_HANDOFF = "mvdrr.hwdef" *) 
module mvdrr
   (ddr4_dimm1_act_n,
    ddr4_dimm1_adr,
    ddr4_dimm1_ba,
    ddr4_dimm1_bg,
    ddr4_dimm1_ck_c,
    ddr4_dimm1_ck_t,
    ddr4_dimm1_cke,
    ddr4_dimm1_cs_n,
    ddr4_dimm1_dm_n,
    ddr4_dimm1_dq,
    ddr4_dimm1_dqs_c,
    ddr4_dimm1_dqs_t,
    ddr4_dimm1_odt,
    ddr4_dimm1_reset_n,
    ddr4_dimm1_sma_clk_clk_n,
    ddr4_dimm1_sma_clk_clk_p);
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 ACT_N" *) (* X_INTERFACE_MODE = "Master" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME ddr4_dimm1, AXI_ARBITRATION_SCHEME TDM, BURST_LENGTH 8, CAN_DEBUG false, CAS_LATENCY 11, CAS_WRITE_LATENCY 11, CS_ENABLED true, DATA_MASK_ENABLED true, DATA_WIDTH 8, MEMORY_TYPE COMPONENTS, MEM_ADDR_MAP ROW_COLUMN_BANK, SLOT Single, TIMEPERIOD_PS 1250" *) output ddr4_dimm1_act_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 ADR" *) output [16:0]ddr4_dimm1_adr;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 BA" *) output [1:0]ddr4_dimm1_ba;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 BG" *) output [1:0]ddr4_dimm1_bg;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 CK_C" *) output ddr4_dimm1_ck_c;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 CK_T" *) output ddr4_dimm1_ck_t;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 CKE" *) output ddr4_dimm1_cke;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 CS_N" *) output ddr4_dimm1_cs_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 DM_N" *) inout [7:0]ddr4_dimm1_dm_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 DQ" *) inout [63:0]ddr4_dimm1_dq;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 DQS_C" *) inout [7:0]ddr4_dimm1_dqs_c;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 DQS_T" *) inout [7:0]ddr4_dimm1_dqs_t;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 ODT" *) output ddr4_dimm1_odt;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddr4:1.0 ddr4_dimm1 RESET_N" *) output ddr4_dimm1_reset_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:diff_clock:1.0 ddr4_dimm1_sma_clk CLK_N" *) (* X_INTERFACE_MODE = "Slave" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME ddr4_dimm1_sma_clk, CAN_DEBUG false, FREQ_HZ 200000000" *) input ddr4_dimm1_sma_clk_clk_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:diff_clock:1.0 ddr4_dimm1_sma_clk CLK_P" *) input ddr4_dimm1_sma_clk_clk_p;

  wire [63:0]axi_dma_0_M_AXIS_MM2S_TDATA;
  wire axi_dma_0_M_AXIS_MM2S_TREADY;
  wire axi_dma_0_M_AXIS_MM2S_TVALID;
  wire [63:0]axi_dma_0_M_AXI_MM2S_ARADDR;
  wire [1:0]axi_dma_0_M_AXI_MM2S_ARBURST;
  wire [3:0]axi_dma_0_M_AXI_MM2S_ARCACHE;
  wire [7:0]axi_dma_0_M_AXI_MM2S_ARLEN;
  wire [2:0]axi_dma_0_M_AXI_MM2S_ARPROT;
  wire [0:0]axi_dma_0_M_AXI_MM2S_ARREADY;
  wire [2:0]axi_dma_0_M_AXI_MM2S_ARSIZE;
  wire axi_dma_0_M_AXI_MM2S_ARVALID;
  wire [63:0]axi_dma_0_M_AXI_MM2S_RDATA;
  wire [0:0]axi_dma_0_M_AXI_MM2S_RLAST;
  wire axi_dma_0_M_AXI_MM2S_RREADY;
  wire [1:0]axi_dma_0_M_AXI_MM2S_RRESP;
  wire [0:0]axi_dma_0_M_AXI_MM2S_RVALID;
  wire axi_dma_0_mm2s_introut;
  wire clk_wizard_0_clk_out1;
  wire clk_wizard_0_locked;
  wire covariance_estimation_0_interrupt;
  wire [63:0]covariance_estimation_0_m_axi_gmem_ARADDR;
  wire [1:0]covariance_estimation_0_m_axi_gmem_ARBURST;
  wire [3:0]covariance_estimation_0_m_axi_gmem_ARCACHE;
  wire [0:0]covariance_estimation_0_m_axi_gmem_ARID;
  wire [7:0]covariance_estimation_0_m_axi_gmem_ARLEN;
  wire [1:0]covariance_estimation_0_m_axi_gmem_ARLOCK;
  wire [2:0]covariance_estimation_0_m_axi_gmem_ARPROT;
  wire [3:0]covariance_estimation_0_m_axi_gmem_ARQOS;
  wire [0:0]covariance_estimation_0_m_axi_gmem_ARREADY;
  wire [3:0]covariance_estimation_0_m_axi_gmem_ARREGION;
  wire [2:0]covariance_estimation_0_m_axi_gmem_ARSIZE;
  wire covariance_estimation_0_m_axi_gmem_ARVALID;
  wire [63:0]covariance_estimation_0_m_axi_gmem_AWADDR;
  wire [1:0]covariance_estimation_0_m_axi_gmem_AWBURST;
  wire [3:0]covariance_estimation_0_m_axi_gmem_AWCACHE;
  wire [0:0]covariance_estimation_0_m_axi_gmem_AWID;
  wire [7:0]covariance_estimation_0_m_axi_gmem_AWLEN;
  wire [1:0]covariance_estimation_0_m_axi_gmem_AWLOCK;
  wire [2:0]covariance_estimation_0_m_axi_gmem_AWPROT;
  wire [3:0]covariance_estimation_0_m_axi_gmem_AWQOS;
  wire [0:0]covariance_estimation_0_m_axi_gmem_AWREADY;
  wire [3:0]covariance_estimation_0_m_axi_gmem_AWREGION;
  wire [2:0]covariance_estimation_0_m_axi_gmem_AWSIZE;
  wire covariance_estimation_0_m_axi_gmem_AWVALID;
  wire [0:0]covariance_estimation_0_m_axi_gmem_BID;
  wire covariance_estimation_0_m_axi_gmem_BREADY;
  wire [1:0]covariance_estimation_0_m_axi_gmem_BRESP;
  wire [0:0]covariance_estimation_0_m_axi_gmem_BVALID;
  wire [63:0]covariance_estimation_0_m_axi_gmem_RDATA;
  wire [0:0]covariance_estimation_0_m_axi_gmem_RID;
  wire [0:0]covariance_estimation_0_m_axi_gmem_RLAST;
  wire covariance_estimation_0_m_axi_gmem_RREADY;
  wire [1:0]covariance_estimation_0_m_axi_gmem_RRESP;
  wire [0:0]covariance_estimation_0_m_axi_gmem_RVALID;
  wire [63:0]covariance_estimation_0_m_axi_gmem_WDATA;
  wire covariance_estimation_0_m_axi_gmem_WLAST;
  wire [0:0]covariance_estimation_0_m_axi_gmem_WREADY;
  wire [7:0]covariance_estimation_0_m_axi_gmem_WSTRB;
  wire covariance_estimation_0_m_axi_gmem_WVALID;
  wire [0:0]\^ddr4_dimm1_act_n ;
  wire [16:0]ddr4_dimm1_adr;
  wire [1:0]ddr4_dimm1_ba;
  wire [1:0]ddr4_dimm1_bg;
  wire [0:0]\^ddr4_dimm1_ck_c ;
  wire [0:0]\^ddr4_dimm1_ck_t ;
  wire [0:0]\^ddr4_dimm1_cke ;
  wire [0:0]\^ddr4_dimm1_cs_n ;
  wire [7:0]ddr4_dimm1_dm_n;
  wire [63:0]ddr4_dimm1_dq;
  wire [7:0]ddr4_dimm1_dqs_c;
  wire [7:0]ddr4_dimm1_dqs_t;
  wire [0:0]\^ddr4_dimm1_odt ;
  wire [0:0]\^ddr4_dimm1_reset_n ;
  wire ddr4_dimm1_sma_clk_clk_n;
  wire ddr4_dimm1_sma_clk_clk_p;
  wire mvdr_weights_0_interrupt;
  wire [63:0]mvdr_weights_0_m_axi_gmem0_ARADDR;
  wire [1:0]mvdr_weights_0_m_axi_gmem0_ARBURST;
  wire [3:0]mvdr_weights_0_m_axi_gmem0_ARCACHE;
  wire [0:0]mvdr_weights_0_m_axi_gmem0_ARID;
  wire [7:0]mvdr_weights_0_m_axi_gmem0_ARLEN;
  wire [1:0]mvdr_weights_0_m_axi_gmem0_ARLOCK;
  wire [2:0]mvdr_weights_0_m_axi_gmem0_ARPROT;
  wire [3:0]mvdr_weights_0_m_axi_gmem0_ARQOS;
  wire mvdr_weights_0_m_axi_gmem0_ARREADY;
  wire [2:0]mvdr_weights_0_m_axi_gmem0_ARSIZE;
  wire mvdr_weights_0_m_axi_gmem0_ARVALID;
  wire [63:0]mvdr_weights_0_m_axi_gmem0_RDATA;
  wire [0:0]mvdr_weights_0_m_axi_gmem0_RID;
  wire mvdr_weights_0_m_axi_gmem0_RLAST;
  wire mvdr_weights_0_m_axi_gmem0_RREADY;
  wire [1:0]mvdr_weights_0_m_axi_gmem0_RRESP;
  wire mvdr_weights_0_m_axi_gmem0_RVALID;
  wire [63:0]mvdr_weights_0_m_axi_gmem1_ARADDR;
  wire [1:0]mvdr_weights_0_m_axi_gmem1_ARBURST;
  wire [3:0]mvdr_weights_0_m_axi_gmem1_ARCACHE;
  wire [0:0]mvdr_weights_0_m_axi_gmem1_ARID;
  wire [7:0]mvdr_weights_0_m_axi_gmem1_ARLEN;
  wire [1:0]mvdr_weights_0_m_axi_gmem1_ARLOCK;
  wire [2:0]mvdr_weights_0_m_axi_gmem1_ARPROT;
  wire [3:0]mvdr_weights_0_m_axi_gmem1_ARQOS;
  wire mvdr_weights_0_m_axi_gmem1_ARREADY;
  wire [2:0]mvdr_weights_0_m_axi_gmem1_ARSIZE;
  wire mvdr_weights_0_m_axi_gmem1_ARVALID;
  wire [63:0]mvdr_weights_0_m_axi_gmem1_RDATA;
  wire [0:0]mvdr_weights_0_m_axi_gmem1_RID;
  wire mvdr_weights_0_m_axi_gmem1_RLAST;
  wire mvdr_weights_0_m_axi_gmem1_RREADY;
  wire [1:0]mvdr_weights_0_m_axi_gmem1_RRESP;
  wire mvdr_weights_0_m_axi_gmem1_RVALID;
  wire [63:0]mvdr_weights_0_m_axi_gmem2_AWADDR;
  wire [1:0]mvdr_weights_0_m_axi_gmem2_AWBURST;
  wire [3:0]mvdr_weights_0_m_axi_gmem2_AWCACHE;
  wire [0:0]mvdr_weights_0_m_axi_gmem2_AWID;
  wire [7:0]mvdr_weights_0_m_axi_gmem2_AWLEN;
  wire [1:0]mvdr_weights_0_m_axi_gmem2_AWLOCK;
  wire [2:0]mvdr_weights_0_m_axi_gmem2_AWPROT;
  wire [3:0]mvdr_weights_0_m_axi_gmem2_AWQOS;
  wire mvdr_weights_0_m_axi_gmem2_AWREADY;
  wire [2:0]mvdr_weights_0_m_axi_gmem2_AWSIZE;
  wire mvdr_weights_0_m_axi_gmem2_AWVALID;
  wire [0:0]mvdr_weights_0_m_axi_gmem2_BID;
  wire mvdr_weights_0_m_axi_gmem2_BREADY;
  wire [1:0]mvdr_weights_0_m_axi_gmem2_BRESP;
  wire mvdr_weights_0_m_axi_gmem2_BVALID;
  wire [63:0]mvdr_weights_0_m_axi_gmem2_WDATA;
  wire mvdr_weights_0_m_axi_gmem2_WLAST;
  wire mvdr_weights_0_m_axi_gmem2_WREADY;
  wire [7:0]mvdr_weights_0_m_axi_gmem2_WSTRB;
  wire mvdr_weights_0_m_axi_gmem2_WVALID;
  wire [0:0]proc_sys_reset_0_peripheral_aresetn;
  wire rdbs_kernel_0_interrupt;
  wire [63:0]rdbs_kernel_0_y_out_TDATA;
  wire rdbs_kernel_0_y_out_TREADY;
  wire rdbs_kernel_0_y_out_TVALID;
  wire [4:0]smartconnect_0_M00_AXI_ARADDR;
  wire smartconnect_0_M00_AXI_ARREADY;
  wire smartconnect_0_M00_AXI_ARVALID;
  wire [4:0]smartconnect_0_M00_AXI_AWADDR;
  wire smartconnect_0_M00_AXI_AWREADY;
  wire smartconnect_0_M00_AXI_AWVALID;
  wire smartconnect_0_M00_AXI_BREADY;
  wire [1:0]smartconnect_0_M00_AXI_BRESP;
  wire smartconnect_0_M00_AXI_BVALID;
  wire [31:0]smartconnect_0_M00_AXI_RDATA;
  wire smartconnect_0_M00_AXI_RREADY;
  wire [1:0]smartconnect_0_M00_AXI_RRESP;
  wire smartconnect_0_M00_AXI_RVALID;
  wire [31:0]smartconnect_0_M00_AXI_WDATA;
  wire smartconnect_0_M00_AXI_WREADY;
  wire [3:0]smartconnect_0_M00_AXI_WSTRB;
  wire smartconnect_0_M00_AXI_WVALID;
  wire [4:0]smartconnect_0_M01_AXI_ARADDR;
  wire smartconnect_0_M01_AXI_ARREADY;
  wire smartconnect_0_M01_AXI_ARVALID;
  wire [4:0]smartconnect_0_M01_AXI_AWADDR;
  wire smartconnect_0_M01_AXI_AWREADY;
  wire smartconnect_0_M01_AXI_AWVALID;
  wire smartconnect_0_M01_AXI_BREADY;
  wire [1:0]smartconnect_0_M01_AXI_BRESP;
  wire smartconnect_0_M01_AXI_BVALID;
  wire [31:0]smartconnect_0_M01_AXI_RDATA;
  wire smartconnect_0_M01_AXI_RREADY;
  wire [1:0]smartconnect_0_M01_AXI_RRESP;
  wire smartconnect_0_M01_AXI_RVALID;
  wire [31:0]smartconnect_0_M01_AXI_WDATA;
  wire smartconnect_0_M01_AXI_WREADY;
  wire [3:0]smartconnect_0_M01_AXI_WSTRB;
  wire smartconnect_0_M01_AXI_WVALID;
  wire [5:0]smartconnect_0_M02_AXI_ARADDR;
  wire smartconnect_0_M02_AXI_ARREADY;
  wire smartconnect_0_M02_AXI_ARVALID;
  wire [5:0]smartconnect_0_M02_AXI_AWADDR;
  wire smartconnect_0_M02_AXI_AWREADY;
  wire smartconnect_0_M02_AXI_AWVALID;
  wire smartconnect_0_M02_AXI_BREADY;
  wire [1:0]smartconnect_0_M02_AXI_BRESP;
  wire smartconnect_0_M02_AXI_BVALID;
  wire [31:0]smartconnect_0_M02_AXI_RDATA;
  wire smartconnect_0_M02_AXI_RREADY;
  wire [1:0]smartconnect_0_M02_AXI_RRESP;
  wire smartconnect_0_M02_AXI_RVALID;
  wire [31:0]smartconnect_0_M02_AXI_WDATA;
  wire smartconnect_0_M02_AXI_WREADY;
  wire [3:0]smartconnect_0_M02_AXI_WSTRB;
  wire smartconnect_0_M02_AXI_WVALID;
  wire [9:0]smartconnect_0_M03_AXI_ARADDR;
  wire smartconnect_0_M03_AXI_ARREADY;
  wire smartconnect_0_M03_AXI_ARVALID;
  wire [9:0]smartconnect_0_M03_AXI_AWADDR;
  wire smartconnect_0_M03_AXI_AWREADY;
  wire smartconnect_0_M03_AXI_AWVALID;
  wire smartconnect_0_M03_AXI_BREADY;
  wire [1:0]smartconnect_0_M03_AXI_BRESP;
  wire smartconnect_0_M03_AXI_BVALID;
  wire [31:0]smartconnect_0_M03_AXI_RDATA;
  wire smartconnect_0_M03_AXI_RREADY;
  wire [1:0]smartconnect_0_M03_AXI_RRESP;
  wire smartconnect_0_M03_AXI_RVALID;
  wire [31:0]smartconnect_0_M03_AXI_WDATA;
  wire smartconnect_0_M03_AXI_WREADY;
  wire smartconnect_0_M03_AXI_WVALID;
  wire [63:0]smartconnect_mem_0_M00_AXI_ARADDR;
  wire [1:0]smartconnect_mem_0_M00_AXI_ARBURST;
  wire [3:0]smartconnect_mem_0_M00_AXI_ARCACHE;
  wire [7:0]smartconnect_mem_0_M00_AXI_ARLEN;
  wire [0:0]smartconnect_mem_0_M00_AXI_ARLOCK;
  wire [2:0]smartconnect_mem_0_M00_AXI_ARPROT;
  wire [3:0]smartconnect_mem_0_M00_AXI_ARQOS;
  wire [0:0]smartconnect_mem_0_M00_AXI_ARREADY;
  wire [2:0]smartconnect_mem_0_M00_AXI_ARSIZE;
  wire smartconnect_mem_0_M00_AXI_ARVALID;
  wire [63:0]smartconnect_mem_0_M00_AXI_AWADDR;
  wire [1:0]smartconnect_mem_0_M00_AXI_AWBURST;
  wire [3:0]smartconnect_mem_0_M00_AXI_AWCACHE;
  wire [7:0]smartconnect_mem_0_M00_AXI_AWLEN;
  wire [0:0]smartconnect_mem_0_M00_AXI_AWLOCK;
  wire [2:0]smartconnect_mem_0_M00_AXI_AWPROT;
  wire [3:0]smartconnect_mem_0_M00_AXI_AWQOS;
  wire [0:0]smartconnect_mem_0_M00_AXI_AWREADY;
  wire [2:0]smartconnect_mem_0_M00_AXI_AWSIZE;
  wire smartconnect_mem_0_M00_AXI_AWVALID;
  wire smartconnect_mem_0_M00_AXI_BREADY;
  wire [1:0]smartconnect_mem_0_M00_AXI_BRESP;
  wire [0:0]smartconnect_mem_0_M00_AXI_BVALID;
  wire [63:0]smartconnect_mem_0_M00_AXI_RDATA;
  wire [0:0]smartconnect_mem_0_M00_AXI_RLAST;
  wire smartconnect_mem_0_M00_AXI_RREADY;
  wire [1:0]smartconnect_mem_0_M00_AXI_RRESP;
  wire [0:0]smartconnect_mem_0_M00_AXI_RVALID;
  wire [63:0]smartconnect_mem_0_M00_AXI_WDATA;
  wire smartconnect_mem_0_M00_AXI_WLAST;
  wire [0:0]smartconnect_mem_0_M00_AXI_WREADY;
  wire [7:0]smartconnect_mem_0_M00_AXI_WSTRB;
  wire smartconnect_mem_0_M00_AXI_WVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_FPD_CCI_NOC_0_ARADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_0_ARBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_0_ARCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_0_ARID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_FPD_CCI_NOC_0_ARLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_0_ARLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_0_ARPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_0_ARQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_0_ARREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_0_ARSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_FPD_CCI_NOC_0_ARUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_0_ARVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_FPD_CCI_NOC_0_AWADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_0_AWBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_0_AWCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_0_AWID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_FPD_CCI_NOC_0_AWLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_0_AWLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_0_AWPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_0_AWQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_0_AWREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_0_AWSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_FPD_CCI_NOC_0_AWUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_0_AWVALID;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_0_BID;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_0_BREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_0_BRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_0_BVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_FPD_CCI_NOC_0_RDATA;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_0_RID;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_0_RLAST;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_0_RREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_0_RRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_0_RVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_FPD_CCI_NOC_0_WDATA;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_0_WLAST;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_0_WREADY;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_0_WSTRB;
  (* HARD_CONN = "true" *) wire [16:0]versal_cips_0_FPD_CCI_NOC_0_WUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_0_WVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_FPD_CCI_NOC_1_ARADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_1_ARBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_1_ARCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_1_ARID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_FPD_CCI_NOC_1_ARLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_1_ARLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_1_ARPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_1_ARQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_1_ARREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_1_ARSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_FPD_CCI_NOC_1_ARUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_1_ARVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_FPD_CCI_NOC_1_AWADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_1_AWBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_1_AWCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_1_AWID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_FPD_CCI_NOC_1_AWLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_1_AWLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_1_AWPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_1_AWQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_1_AWREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_1_AWSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_FPD_CCI_NOC_1_AWUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_1_AWVALID;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_1_BID;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_1_BREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_1_BRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_1_BVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_FPD_CCI_NOC_1_RDATA;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_1_RID;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_1_RLAST;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_1_RREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_1_RRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_1_RVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_FPD_CCI_NOC_1_WDATA;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_1_WLAST;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_1_WREADY;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_1_WSTRB;
  (* HARD_CONN = "true" *) wire [16:0]versal_cips_0_FPD_CCI_NOC_1_WUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_1_WVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_FPD_CCI_NOC_2_ARADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_2_ARBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_2_ARCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_2_ARID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_FPD_CCI_NOC_2_ARLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_2_ARLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_2_ARPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_2_ARQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_2_ARREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_2_ARSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_FPD_CCI_NOC_2_ARUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_2_ARVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_FPD_CCI_NOC_2_AWADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_2_AWBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_2_AWCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_2_AWID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_FPD_CCI_NOC_2_AWLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_2_AWLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_2_AWPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_2_AWQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_2_AWREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_2_AWSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_FPD_CCI_NOC_2_AWUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_2_AWVALID;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_2_BID;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_2_BREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_2_BRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_2_BVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_FPD_CCI_NOC_2_RDATA;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_2_RID;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_2_RLAST;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_2_RREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_2_RRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_2_RVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_FPD_CCI_NOC_2_WDATA;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_2_WLAST;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_2_WREADY;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_2_WSTRB;
  (* HARD_CONN = "true" *) wire [16:0]versal_cips_0_FPD_CCI_NOC_2_WUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_2_WVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_FPD_CCI_NOC_3_ARADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_3_ARBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_3_ARCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_3_ARID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_FPD_CCI_NOC_3_ARLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_3_ARLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_3_ARPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_3_ARQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_3_ARREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_3_ARSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_FPD_CCI_NOC_3_ARUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_3_ARVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_FPD_CCI_NOC_3_AWADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_3_AWBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_3_AWCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_3_AWID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_FPD_CCI_NOC_3_AWLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_3_AWLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_3_AWPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_FPD_CCI_NOC_3_AWQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_3_AWREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_FPD_CCI_NOC_3_AWSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_FPD_CCI_NOC_3_AWUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_3_AWVALID;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_3_BID;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_3_BREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_3_BRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_3_BVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_FPD_CCI_NOC_3_RDATA;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_3_RID;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_3_RLAST;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_3_RREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_FPD_CCI_NOC_3_RRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_3_RVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_FPD_CCI_NOC_3_WDATA;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_3_WLAST;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_FPD_CCI_NOC_3_WREADY;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_FPD_CCI_NOC_3_WSTRB;
  (* HARD_CONN = "true" *) wire [16:0]versal_cips_0_FPD_CCI_NOC_3_WUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_FPD_CCI_NOC_3_WVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_LPD_AXI_NOC_0_ARADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_LPD_AXI_NOC_0_ARBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_LPD_AXI_NOC_0_ARCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_LPD_AXI_NOC_0_ARID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_LPD_AXI_NOC_0_ARLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_LPD_AXI_NOC_0_ARLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_LPD_AXI_NOC_0_ARPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_LPD_AXI_NOC_0_ARQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_LPD_AXI_NOC_0_ARREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_LPD_AXI_NOC_0_ARSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_LPD_AXI_NOC_0_ARUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_LPD_AXI_NOC_0_ARVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_LPD_AXI_NOC_0_AWADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_LPD_AXI_NOC_0_AWBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_LPD_AXI_NOC_0_AWCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_LPD_AXI_NOC_0_AWID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_LPD_AXI_NOC_0_AWLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_LPD_AXI_NOC_0_AWLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_LPD_AXI_NOC_0_AWPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_LPD_AXI_NOC_0_AWQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_LPD_AXI_NOC_0_AWREADY;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_LPD_AXI_NOC_0_AWSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_LPD_AXI_NOC_0_AWUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_LPD_AXI_NOC_0_AWVALID;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_LPD_AXI_NOC_0_BID;
  (* HARD_CONN = "true" *) wire versal_cips_0_LPD_AXI_NOC_0_BREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_LPD_AXI_NOC_0_BRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_LPD_AXI_NOC_0_BVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_LPD_AXI_NOC_0_RDATA;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_LPD_AXI_NOC_0_RID;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_LPD_AXI_NOC_0_RLAST;
  (* HARD_CONN = "true" *) wire versal_cips_0_LPD_AXI_NOC_0_RREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_LPD_AXI_NOC_0_RRESP;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_LPD_AXI_NOC_0_RVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_LPD_AXI_NOC_0_WDATA;
  (* HARD_CONN = "true" *) wire versal_cips_0_LPD_AXI_NOC_0_WLAST;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_LPD_AXI_NOC_0_WREADY;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_LPD_AXI_NOC_0_WSTRB;
  (* HARD_CONN = "true" *) wire versal_cips_0_LPD_AXI_NOC_0_WVALID;
  wire [43:0]versal_cips_0_M_AXI_FPD_ARADDR;
  wire [1:0]versal_cips_0_M_AXI_FPD_ARBURST;
  wire [3:0]versal_cips_0_M_AXI_FPD_ARCACHE;
  wire [15:0]versal_cips_0_M_AXI_FPD_ARID;
  wire [7:0]versal_cips_0_M_AXI_FPD_ARLEN;
  wire versal_cips_0_M_AXI_FPD_ARLOCK;
  wire [2:0]versal_cips_0_M_AXI_FPD_ARPROT;
  wire [3:0]versal_cips_0_M_AXI_FPD_ARQOS;
  wire versal_cips_0_M_AXI_FPD_ARREADY;
  wire [2:0]versal_cips_0_M_AXI_FPD_ARSIZE;
  wire [15:0]versal_cips_0_M_AXI_FPD_ARUSER;
  wire versal_cips_0_M_AXI_FPD_ARVALID;
  wire [43:0]versal_cips_0_M_AXI_FPD_AWADDR;
  wire [1:0]versal_cips_0_M_AXI_FPD_AWBURST;
  wire [3:0]versal_cips_0_M_AXI_FPD_AWCACHE;
  wire [15:0]versal_cips_0_M_AXI_FPD_AWID;
  wire [7:0]versal_cips_0_M_AXI_FPD_AWLEN;
  wire versal_cips_0_M_AXI_FPD_AWLOCK;
  wire [2:0]versal_cips_0_M_AXI_FPD_AWPROT;
  wire [3:0]versal_cips_0_M_AXI_FPD_AWQOS;
  wire versal_cips_0_M_AXI_FPD_AWREADY;
  wire [2:0]versal_cips_0_M_AXI_FPD_AWSIZE;
  wire [15:0]versal_cips_0_M_AXI_FPD_AWUSER;
  wire versal_cips_0_M_AXI_FPD_AWVALID;
  wire [15:0]versal_cips_0_M_AXI_FPD_BID;
  wire versal_cips_0_M_AXI_FPD_BREADY;
  wire [1:0]versal_cips_0_M_AXI_FPD_BRESP;
  wire versal_cips_0_M_AXI_FPD_BVALID;
  wire [127:0]versal_cips_0_M_AXI_FPD_RDATA;
  wire [15:0]versal_cips_0_M_AXI_FPD_RID;
  wire versal_cips_0_M_AXI_FPD_RLAST;
  wire versal_cips_0_M_AXI_FPD_RREADY;
  wire [1:0]versal_cips_0_M_AXI_FPD_RRESP;
  wire versal_cips_0_M_AXI_FPD_RVALID;
  wire [127:0]versal_cips_0_M_AXI_FPD_WDATA;
  wire versal_cips_0_M_AXI_FPD_WLAST;
  wire versal_cips_0_M_AXI_FPD_WREADY;
  wire [15:0]versal_cips_0_M_AXI_FPD_WSTRB;
  wire versal_cips_0_M_AXI_FPD_WVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_PMC_NOC_AXI_0_ARADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_PMC_NOC_AXI_0_ARBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_PMC_NOC_AXI_0_ARCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_PMC_NOC_AXI_0_ARID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_PMC_NOC_AXI_0_ARLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_PMC_NOC_AXI_0_ARLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_PMC_NOC_AXI_0_ARPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_PMC_NOC_AXI_0_ARQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_PMC_NOC_AXI_0_ARREADY;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_PMC_NOC_AXI_0_ARREGION;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_PMC_NOC_AXI_0_ARSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_PMC_NOC_AXI_0_ARUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_PMC_NOC_AXI_0_ARVALID;
  (* HARD_CONN = "true" *) wire [63:0]versal_cips_0_PMC_NOC_AXI_0_AWADDR;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_PMC_NOC_AXI_0_AWBURST;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_PMC_NOC_AXI_0_AWCACHE;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_PMC_NOC_AXI_0_AWID;
  (* HARD_CONN = "true" *) wire [7:0]versal_cips_0_PMC_NOC_AXI_0_AWLEN;
  (* HARD_CONN = "true" *) wire versal_cips_0_PMC_NOC_AXI_0_AWLOCK;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_PMC_NOC_AXI_0_AWPROT;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_PMC_NOC_AXI_0_AWQOS;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_PMC_NOC_AXI_0_AWREADY;
  (* HARD_CONN = "true" *) wire [3:0]versal_cips_0_PMC_NOC_AXI_0_AWREGION;
  (* HARD_CONN = "true" *) wire [2:0]versal_cips_0_PMC_NOC_AXI_0_AWSIZE;
  (* HARD_CONN = "true" *) wire [17:0]versal_cips_0_PMC_NOC_AXI_0_AWUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_PMC_NOC_AXI_0_AWVALID;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_PMC_NOC_AXI_0_BID;
  (* HARD_CONN = "true" *) wire versal_cips_0_PMC_NOC_AXI_0_BREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_PMC_NOC_AXI_0_BRESP;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_PMC_NOC_AXI_0_BUSER;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_PMC_NOC_AXI_0_BVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_PMC_NOC_AXI_0_RDATA;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_PMC_NOC_AXI_0_RID;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_PMC_NOC_AXI_0_RLAST;
  (* HARD_CONN = "true" *) wire versal_cips_0_PMC_NOC_AXI_0_RREADY;
  (* HARD_CONN = "true" *) wire [1:0]versal_cips_0_PMC_NOC_AXI_0_RRESP;
  (* HARD_CONN = "true" *) wire [16:0]versal_cips_0_PMC_NOC_AXI_0_RUSER;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_PMC_NOC_AXI_0_RVALID;
  (* HARD_CONN = "true" *) wire [127:0]versal_cips_0_PMC_NOC_AXI_0_WDATA;
  (* HARD_CONN = "true" *) wire versal_cips_0_PMC_NOC_AXI_0_WLAST;
  (* HARD_CONN = "true" *) wire [0:0]versal_cips_0_PMC_NOC_AXI_0_WREADY;
  (* HARD_CONN = "true" *) wire [15:0]versal_cips_0_PMC_NOC_AXI_0_WSTRB;
  (* HARD_CONN = "true" *) wire [16:0]versal_cips_0_PMC_NOC_AXI_0_WUSER;
  (* HARD_CONN = "true" *) wire versal_cips_0_PMC_NOC_AXI_0_WVALID;
  (* HARD_CONN = "true" *) wire versal_cips_0_fpd_cci_noc_axi0_clk;
  (* HARD_CONN = "true" *) wire versal_cips_0_fpd_cci_noc_axi1_clk;
  (* HARD_CONN = "true" *) wire versal_cips_0_fpd_cci_noc_axi2_clk;
  (* HARD_CONN = "true" *) wire versal_cips_0_fpd_cci_noc_axi3_clk;
  (* HARD_CONN = "true" *) wire versal_cips_0_lpd_axi_noc_clk;
  wire versal_cips_0_pl0_ref_clk;
  wire versal_cips_0_pl0_resetn;
  (* HARD_CONN = "true" *) wire versal_cips_0_pmc_axi_noc_axi0_clk;

  assign ddr4_dimm1_act_n = \^ddr4_dimm1_act_n [0];
  assign ddr4_dimm1_ck_c = \^ddr4_dimm1_ck_c [0];
  assign ddr4_dimm1_ck_t = \^ddr4_dimm1_ck_t [0];
  assign ddr4_dimm1_cke = \^ddr4_dimm1_cke [0];
  assign ddr4_dimm1_cs_n = \^ddr4_dimm1_cs_n [0];
  assign ddr4_dimm1_odt = \^ddr4_dimm1_odt [0];
  assign ddr4_dimm1_reset_n = \^ddr4_dimm1_reset_n [0];
  mvdrr_axi_dma_0_0 axi_dma_0
       (.axi_resetn(proc_sys_reset_0_peripheral_aresetn),
        .m_axi_mm2s_aclk(clk_wizard_0_clk_out1),
        .m_axi_mm2s_araddr(axi_dma_0_M_AXI_MM2S_ARADDR),
        .m_axi_mm2s_arburst(axi_dma_0_M_AXI_MM2S_ARBURST),
        .m_axi_mm2s_arcache(axi_dma_0_M_AXI_MM2S_ARCACHE),
        .m_axi_mm2s_arlen(axi_dma_0_M_AXI_MM2S_ARLEN),
        .m_axi_mm2s_arprot(axi_dma_0_M_AXI_MM2S_ARPROT),
        .m_axi_mm2s_arready(axi_dma_0_M_AXI_MM2S_ARREADY),
        .m_axi_mm2s_arsize(axi_dma_0_M_AXI_MM2S_ARSIZE),
        .m_axi_mm2s_arvalid(axi_dma_0_M_AXI_MM2S_ARVALID),
        .m_axi_mm2s_rdata(axi_dma_0_M_AXI_MM2S_RDATA),
        .m_axi_mm2s_rlast(axi_dma_0_M_AXI_MM2S_RLAST),
        .m_axi_mm2s_rready(axi_dma_0_M_AXI_MM2S_RREADY),
        .m_axi_mm2s_rresp(axi_dma_0_M_AXI_MM2S_RRESP),
        .m_axi_mm2s_rvalid(axi_dma_0_M_AXI_MM2S_RVALID),
        .m_axis_mm2s_tdata(axi_dma_0_M_AXIS_MM2S_TDATA),
        .m_axis_mm2s_tready(axi_dma_0_M_AXIS_MM2S_TREADY),
        .m_axis_mm2s_tvalid(axi_dma_0_M_AXIS_MM2S_TVALID),
        .mm2s_introut(axi_dma_0_mm2s_introut),
        .s_axi_lite_aclk(clk_wizard_0_clk_out1),
        .s_axi_lite_araddr(smartconnect_0_M03_AXI_ARADDR),
        .s_axi_lite_arready(smartconnect_0_M03_AXI_ARREADY),
        .s_axi_lite_arvalid(smartconnect_0_M03_AXI_ARVALID),
        .s_axi_lite_awaddr(smartconnect_0_M03_AXI_AWADDR),
        .s_axi_lite_awready(smartconnect_0_M03_AXI_AWREADY),
        .s_axi_lite_awvalid(smartconnect_0_M03_AXI_AWVALID),
        .s_axi_lite_bready(smartconnect_0_M03_AXI_BREADY),
        .s_axi_lite_bresp(smartconnect_0_M03_AXI_BRESP),
        .s_axi_lite_bvalid(smartconnect_0_M03_AXI_BVALID),
        .s_axi_lite_rdata(smartconnect_0_M03_AXI_RDATA),
        .s_axi_lite_rready(smartconnect_0_M03_AXI_RREADY),
        .s_axi_lite_rresp(smartconnect_0_M03_AXI_RRESP),
        .s_axi_lite_rvalid(smartconnect_0_M03_AXI_RVALID),
        .s_axi_lite_wdata(smartconnect_0_M03_AXI_WDATA),
        .s_axi_lite_wready(smartconnect_0_M03_AXI_WREADY),
        .s_axi_lite_wvalid(smartconnect_0_M03_AXI_WVALID));
  mvdrr_axi_noc_0_0 axi_noc_0
       (.CH0_DDR4_0_act_n(\^ddr4_dimm1_act_n ),
        .CH0_DDR4_0_adr(ddr4_dimm1_adr),
        .CH0_DDR4_0_ba(ddr4_dimm1_ba),
        .CH0_DDR4_0_bg(ddr4_dimm1_bg),
        .CH0_DDR4_0_ck_c(\^ddr4_dimm1_ck_c ),
        .CH0_DDR4_0_ck_t(\^ddr4_dimm1_ck_t ),
        .CH0_DDR4_0_cke(\^ddr4_dimm1_cke ),
        .CH0_DDR4_0_cs_n(\^ddr4_dimm1_cs_n ),
        .CH0_DDR4_0_dm_n(ddr4_dimm1_dm_n),
        .CH0_DDR4_0_dq(ddr4_dimm1_dq),
        .CH0_DDR4_0_dqs_c(ddr4_dimm1_dqs_c),
        .CH0_DDR4_0_dqs_t(ddr4_dimm1_dqs_t),
        .CH0_DDR4_0_odt(\^ddr4_dimm1_odt ),
        .CH0_DDR4_0_reset_n(\^ddr4_dimm1_reset_n ),
        .S00_AXI_araddr(versal_cips_0_FPD_CCI_NOC_0_ARADDR),
        .S00_AXI_arburst(versal_cips_0_FPD_CCI_NOC_0_ARBURST),
        .S00_AXI_arcache(versal_cips_0_FPD_CCI_NOC_0_ARCACHE),
        .S00_AXI_arid(versal_cips_0_FPD_CCI_NOC_0_ARID),
        .S00_AXI_arlen(versal_cips_0_FPD_CCI_NOC_0_ARLEN),
        .S00_AXI_arlock(versal_cips_0_FPD_CCI_NOC_0_ARLOCK),
        .S00_AXI_arprot(versal_cips_0_FPD_CCI_NOC_0_ARPROT),
        .S00_AXI_arqos(versal_cips_0_FPD_CCI_NOC_0_ARQOS),
        .S00_AXI_arready(versal_cips_0_FPD_CCI_NOC_0_ARREADY),
        .S00_AXI_arregion({1'b0,1'b0,1'b0,1'b0}),
        .S00_AXI_arsize(versal_cips_0_FPD_CCI_NOC_0_ARSIZE),
        .S00_AXI_aruser(versal_cips_0_FPD_CCI_NOC_0_ARUSER),
        .S00_AXI_arvalid(versal_cips_0_FPD_CCI_NOC_0_ARVALID),
        .S00_AXI_awaddr(versal_cips_0_FPD_CCI_NOC_0_AWADDR),
        .S00_AXI_awburst(versal_cips_0_FPD_CCI_NOC_0_AWBURST),
        .S00_AXI_awcache(versal_cips_0_FPD_CCI_NOC_0_AWCACHE),
        .S00_AXI_awid(versal_cips_0_FPD_CCI_NOC_0_AWID),
        .S00_AXI_awlen(versal_cips_0_FPD_CCI_NOC_0_AWLEN),
        .S00_AXI_awlock(versal_cips_0_FPD_CCI_NOC_0_AWLOCK),
        .S00_AXI_awprot(versal_cips_0_FPD_CCI_NOC_0_AWPROT),
        .S00_AXI_awqos(versal_cips_0_FPD_CCI_NOC_0_AWQOS),
        .S00_AXI_awready(versal_cips_0_FPD_CCI_NOC_0_AWREADY),
        .S00_AXI_awregion({1'b0,1'b0,1'b0,1'b0}),
        .S00_AXI_awsize(versal_cips_0_FPD_CCI_NOC_0_AWSIZE),
        .S00_AXI_awuser(versal_cips_0_FPD_CCI_NOC_0_AWUSER),
        .S00_AXI_awvalid(versal_cips_0_FPD_CCI_NOC_0_AWVALID),
        .S00_AXI_bid(versal_cips_0_FPD_CCI_NOC_0_BID),
        .S00_AXI_bready(versal_cips_0_FPD_CCI_NOC_0_BREADY),
        .S00_AXI_bresp(versal_cips_0_FPD_CCI_NOC_0_BRESP),
        .S00_AXI_bvalid(versal_cips_0_FPD_CCI_NOC_0_BVALID),
        .S00_AXI_rdata(versal_cips_0_FPD_CCI_NOC_0_RDATA),
        .S00_AXI_rid(versal_cips_0_FPD_CCI_NOC_0_RID),
        .S00_AXI_rlast(versal_cips_0_FPD_CCI_NOC_0_RLAST),
        .S00_AXI_rready(versal_cips_0_FPD_CCI_NOC_0_RREADY),
        .S00_AXI_rresp(versal_cips_0_FPD_CCI_NOC_0_RRESP),
        .S00_AXI_rvalid(versal_cips_0_FPD_CCI_NOC_0_RVALID),
        .S00_AXI_wdata(versal_cips_0_FPD_CCI_NOC_0_WDATA),
        .S00_AXI_wlast(versal_cips_0_FPD_CCI_NOC_0_WLAST),
        .S00_AXI_wready(versal_cips_0_FPD_CCI_NOC_0_WREADY),
        .S00_AXI_wstrb(versal_cips_0_FPD_CCI_NOC_0_WSTRB),
        .S00_AXI_wuser(versal_cips_0_FPD_CCI_NOC_0_WUSER),
        .S00_AXI_wvalid(versal_cips_0_FPD_CCI_NOC_0_WVALID),
        .S01_AXI_araddr(versal_cips_0_FPD_CCI_NOC_1_ARADDR),
        .S01_AXI_arburst(versal_cips_0_FPD_CCI_NOC_1_ARBURST),
        .S01_AXI_arcache(versal_cips_0_FPD_CCI_NOC_1_ARCACHE),
        .S01_AXI_arid(versal_cips_0_FPD_CCI_NOC_1_ARID),
        .S01_AXI_arlen(versal_cips_0_FPD_CCI_NOC_1_ARLEN),
        .S01_AXI_arlock(versal_cips_0_FPD_CCI_NOC_1_ARLOCK),
        .S01_AXI_arprot(versal_cips_0_FPD_CCI_NOC_1_ARPROT),
        .S01_AXI_arqos(versal_cips_0_FPD_CCI_NOC_1_ARQOS),
        .S01_AXI_arready(versal_cips_0_FPD_CCI_NOC_1_ARREADY),
        .S01_AXI_arregion({1'b0,1'b0,1'b0,1'b0}),
        .S01_AXI_arsize(versal_cips_0_FPD_CCI_NOC_1_ARSIZE),
        .S01_AXI_aruser(versal_cips_0_FPD_CCI_NOC_1_ARUSER),
        .S01_AXI_arvalid(versal_cips_0_FPD_CCI_NOC_1_ARVALID),
        .S01_AXI_awaddr(versal_cips_0_FPD_CCI_NOC_1_AWADDR),
        .S01_AXI_awburst(versal_cips_0_FPD_CCI_NOC_1_AWBURST),
        .S01_AXI_awcache(versal_cips_0_FPD_CCI_NOC_1_AWCACHE),
        .S01_AXI_awid(versal_cips_0_FPD_CCI_NOC_1_AWID),
        .S01_AXI_awlen(versal_cips_0_FPD_CCI_NOC_1_AWLEN),
        .S01_AXI_awlock(versal_cips_0_FPD_CCI_NOC_1_AWLOCK),
        .S01_AXI_awprot(versal_cips_0_FPD_CCI_NOC_1_AWPROT),
        .S01_AXI_awqos(versal_cips_0_FPD_CCI_NOC_1_AWQOS),
        .S01_AXI_awready(versal_cips_0_FPD_CCI_NOC_1_AWREADY),
        .S01_AXI_awregion({1'b0,1'b0,1'b0,1'b0}),
        .S01_AXI_awsize(versal_cips_0_FPD_CCI_NOC_1_AWSIZE),
        .S01_AXI_awuser(versal_cips_0_FPD_CCI_NOC_1_AWUSER),
        .S01_AXI_awvalid(versal_cips_0_FPD_CCI_NOC_1_AWVALID),
        .S01_AXI_bid(versal_cips_0_FPD_CCI_NOC_1_BID),
        .S01_AXI_bready(versal_cips_0_FPD_CCI_NOC_1_BREADY),
        .S01_AXI_bresp(versal_cips_0_FPD_CCI_NOC_1_BRESP),
        .S01_AXI_bvalid(versal_cips_0_FPD_CCI_NOC_1_BVALID),
        .S01_AXI_rdata(versal_cips_0_FPD_CCI_NOC_1_RDATA),
        .S01_AXI_rid(versal_cips_0_FPD_CCI_NOC_1_RID),
        .S01_AXI_rlast(versal_cips_0_FPD_CCI_NOC_1_RLAST),
        .S01_AXI_rready(versal_cips_0_FPD_CCI_NOC_1_RREADY),
        .S01_AXI_rresp(versal_cips_0_FPD_CCI_NOC_1_RRESP),
        .S01_AXI_rvalid(versal_cips_0_FPD_CCI_NOC_1_RVALID),
        .S01_AXI_wdata(versal_cips_0_FPD_CCI_NOC_1_WDATA),
        .S01_AXI_wlast(versal_cips_0_FPD_CCI_NOC_1_WLAST),
        .S01_AXI_wready(versal_cips_0_FPD_CCI_NOC_1_WREADY),
        .S01_AXI_wstrb(versal_cips_0_FPD_CCI_NOC_1_WSTRB),
        .S01_AXI_wuser(versal_cips_0_FPD_CCI_NOC_1_WUSER),
        .S01_AXI_wvalid(versal_cips_0_FPD_CCI_NOC_1_WVALID),
        .S02_AXI_araddr(versal_cips_0_FPD_CCI_NOC_2_ARADDR),
        .S02_AXI_arburst(versal_cips_0_FPD_CCI_NOC_2_ARBURST),
        .S02_AXI_arcache(versal_cips_0_FPD_CCI_NOC_2_ARCACHE),
        .S02_AXI_arid(versal_cips_0_FPD_CCI_NOC_2_ARID),
        .S02_AXI_arlen(versal_cips_0_FPD_CCI_NOC_2_ARLEN),
        .S02_AXI_arlock(versal_cips_0_FPD_CCI_NOC_2_ARLOCK),
        .S02_AXI_arprot(versal_cips_0_FPD_CCI_NOC_2_ARPROT),
        .S02_AXI_arqos(versal_cips_0_FPD_CCI_NOC_2_ARQOS),
        .S02_AXI_arready(versal_cips_0_FPD_CCI_NOC_2_ARREADY),
        .S02_AXI_arregion({1'b0,1'b0,1'b0,1'b0}),
        .S02_AXI_arsize(versal_cips_0_FPD_CCI_NOC_2_ARSIZE),
        .S02_AXI_aruser(versal_cips_0_FPD_CCI_NOC_2_ARUSER),
        .S02_AXI_arvalid(versal_cips_0_FPD_CCI_NOC_2_ARVALID),
        .S02_AXI_awaddr(versal_cips_0_FPD_CCI_NOC_2_AWADDR),
        .S02_AXI_awburst(versal_cips_0_FPD_CCI_NOC_2_AWBURST),
        .S02_AXI_awcache(versal_cips_0_FPD_CCI_NOC_2_AWCACHE),
        .S02_AXI_awid(versal_cips_0_FPD_CCI_NOC_2_AWID),
        .S02_AXI_awlen(versal_cips_0_FPD_CCI_NOC_2_AWLEN),
        .S02_AXI_awlock(versal_cips_0_FPD_CCI_NOC_2_AWLOCK),
        .S02_AXI_awprot(versal_cips_0_FPD_CCI_NOC_2_AWPROT),
        .S02_AXI_awqos(versal_cips_0_FPD_CCI_NOC_2_AWQOS),
        .S02_AXI_awready(versal_cips_0_FPD_CCI_NOC_2_AWREADY),
        .S02_AXI_awregion({1'b0,1'b0,1'b0,1'b0}),
        .S02_AXI_awsize(versal_cips_0_FPD_CCI_NOC_2_AWSIZE),
        .S02_AXI_awuser(versal_cips_0_FPD_CCI_NOC_2_AWUSER),
        .S02_AXI_awvalid(versal_cips_0_FPD_CCI_NOC_2_AWVALID),
        .S02_AXI_bid(versal_cips_0_FPD_CCI_NOC_2_BID),
        .S02_AXI_bready(versal_cips_0_FPD_CCI_NOC_2_BREADY),
        .S02_AXI_bresp(versal_cips_0_FPD_CCI_NOC_2_BRESP),
        .S02_AXI_bvalid(versal_cips_0_FPD_CCI_NOC_2_BVALID),
        .S02_AXI_rdata(versal_cips_0_FPD_CCI_NOC_2_RDATA),
        .S02_AXI_rid(versal_cips_0_FPD_CCI_NOC_2_RID),
        .S02_AXI_rlast(versal_cips_0_FPD_CCI_NOC_2_RLAST),
        .S02_AXI_rready(versal_cips_0_FPD_CCI_NOC_2_RREADY),
        .S02_AXI_rresp(versal_cips_0_FPD_CCI_NOC_2_RRESP),
        .S02_AXI_rvalid(versal_cips_0_FPD_CCI_NOC_2_RVALID),
        .S02_AXI_wdata(versal_cips_0_FPD_CCI_NOC_2_WDATA),
        .S02_AXI_wlast(versal_cips_0_FPD_CCI_NOC_2_WLAST),
        .S02_AXI_wready(versal_cips_0_FPD_CCI_NOC_2_WREADY),
        .S02_AXI_wstrb(versal_cips_0_FPD_CCI_NOC_2_WSTRB),
        .S02_AXI_wuser(versal_cips_0_FPD_CCI_NOC_2_WUSER),
        .S02_AXI_wvalid(versal_cips_0_FPD_CCI_NOC_2_WVALID),
        .S03_AXI_araddr(versal_cips_0_FPD_CCI_NOC_3_ARADDR),
        .S03_AXI_arburst(versal_cips_0_FPD_CCI_NOC_3_ARBURST),
        .S03_AXI_arcache(versal_cips_0_FPD_CCI_NOC_3_ARCACHE),
        .S03_AXI_arid(versal_cips_0_FPD_CCI_NOC_3_ARID),
        .S03_AXI_arlen(versal_cips_0_FPD_CCI_NOC_3_ARLEN),
        .S03_AXI_arlock(versal_cips_0_FPD_CCI_NOC_3_ARLOCK),
        .S03_AXI_arprot(versal_cips_0_FPD_CCI_NOC_3_ARPROT),
        .S03_AXI_arqos(versal_cips_0_FPD_CCI_NOC_3_ARQOS),
        .S03_AXI_arready(versal_cips_0_FPD_CCI_NOC_3_ARREADY),
        .S03_AXI_arregion({1'b0,1'b0,1'b0,1'b0}),
        .S03_AXI_arsize(versal_cips_0_FPD_CCI_NOC_3_ARSIZE),
        .S03_AXI_aruser(versal_cips_0_FPD_CCI_NOC_3_ARUSER),
        .S03_AXI_arvalid(versal_cips_0_FPD_CCI_NOC_3_ARVALID),
        .S03_AXI_awaddr(versal_cips_0_FPD_CCI_NOC_3_AWADDR),
        .S03_AXI_awburst(versal_cips_0_FPD_CCI_NOC_3_AWBURST),
        .S03_AXI_awcache(versal_cips_0_FPD_CCI_NOC_3_AWCACHE),
        .S03_AXI_awid(versal_cips_0_FPD_CCI_NOC_3_AWID),
        .S03_AXI_awlen(versal_cips_0_FPD_CCI_NOC_3_AWLEN),
        .S03_AXI_awlock(versal_cips_0_FPD_CCI_NOC_3_AWLOCK),
        .S03_AXI_awprot(versal_cips_0_FPD_CCI_NOC_3_AWPROT),
        .S03_AXI_awqos(versal_cips_0_FPD_CCI_NOC_3_AWQOS),
        .S03_AXI_awready(versal_cips_0_FPD_CCI_NOC_3_AWREADY),
        .S03_AXI_awregion({1'b0,1'b0,1'b0,1'b0}),
        .S03_AXI_awsize(versal_cips_0_FPD_CCI_NOC_3_AWSIZE),
        .S03_AXI_awuser(versal_cips_0_FPD_CCI_NOC_3_AWUSER),
        .S03_AXI_awvalid(versal_cips_0_FPD_CCI_NOC_3_AWVALID),
        .S03_AXI_bid(versal_cips_0_FPD_CCI_NOC_3_BID),
        .S03_AXI_bready(versal_cips_0_FPD_CCI_NOC_3_BREADY),
        .S03_AXI_bresp(versal_cips_0_FPD_CCI_NOC_3_BRESP),
        .S03_AXI_bvalid(versal_cips_0_FPD_CCI_NOC_3_BVALID),
        .S03_AXI_rdata(versal_cips_0_FPD_CCI_NOC_3_RDATA),
        .S03_AXI_rid(versal_cips_0_FPD_CCI_NOC_3_RID),
        .S03_AXI_rlast(versal_cips_0_FPD_CCI_NOC_3_RLAST),
        .S03_AXI_rready(versal_cips_0_FPD_CCI_NOC_3_RREADY),
        .S03_AXI_rresp(versal_cips_0_FPD_CCI_NOC_3_RRESP),
        .S03_AXI_rvalid(versal_cips_0_FPD_CCI_NOC_3_RVALID),
        .S03_AXI_wdata(versal_cips_0_FPD_CCI_NOC_3_WDATA),
        .S03_AXI_wlast(versal_cips_0_FPD_CCI_NOC_3_WLAST),
        .S03_AXI_wready(versal_cips_0_FPD_CCI_NOC_3_WREADY),
        .S03_AXI_wstrb(versal_cips_0_FPD_CCI_NOC_3_WSTRB),
        .S03_AXI_wuser(versal_cips_0_FPD_CCI_NOC_3_WUSER),
        .S03_AXI_wvalid(versal_cips_0_FPD_CCI_NOC_3_WVALID),
        .S04_AXI_araddr(versal_cips_0_LPD_AXI_NOC_0_ARADDR),
        .S04_AXI_arburst(versal_cips_0_LPD_AXI_NOC_0_ARBURST),
        .S04_AXI_arcache(versal_cips_0_LPD_AXI_NOC_0_ARCACHE),
        .S04_AXI_arid(versal_cips_0_LPD_AXI_NOC_0_ARID),
        .S04_AXI_arlen(versal_cips_0_LPD_AXI_NOC_0_ARLEN),
        .S04_AXI_arlock(versal_cips_0_LPD_AXI_NOC_0_ARLOCK),
        .S04_AXI_arprot(versal_cips_0_LPD_AXI_NOC_0_ARPROT),
        .S04_AXI_arqos(versal_cips_0_LPD_AXI_NOC_0_ARQOS),
        .S04_AXI_arready(versal_cips_0_LPD_AXI_NOC_0_ARREADY),
        .S04_AXI_arregion({1'b0,1'b0,1'b0,1'b0}),
        .S04_AXI_arsize(versal_cips_0_LPD_AXI_NOC_0_ARSIZE),
        .S04_AXI_aruser(versal_cips_0_LPD_AXI_NOC_0_ARUSER),
        .S04_AXI_arvalid(versal_cips_0_LPD_AXI_NOC_0_ARVALID),
        .S04_AXI_awaddr(versal_cips_0_LPD_AXI_NOC_0_AWADDR),
        .S04_AXI_awburst(versal_cips_0_LPD_AXI_NOC_0_AWBURST),
        .S04_AXI_awcache(versal_cips_0_LPD_AXI_NOC_0_AWCACHE),
        .S04_AXI_awid(versal_cips_0_LPD_AXI_NOC_0_AWID),
        .S04_AXI_awlen(versal_cips_0_LPD_AXI_NOC_0_AWLEN),
        .S04_AXI_awlock(versal_cips_0_LPD_AXI_NOC_0_AWLOCK),
        .S04_AXI_awprot(versal_cips_0_LPD_AXI_NOC_0_AWPROT),
        .S04_AXI_awqos(versal_cips_0_LPD_AXI_NOC_0_AWQOS),
        .S04_AXI_awready(versal_cips_0_LPD_AXI_NOC_0_AWREADY),
        .S04_AXI_awregion({1'b0,1'b0,1'b0,1'b0}),
        .S04_AXI_awsize(versal_cips_0_LPD_AXI_NOC_0_AWSIZE),
        .S04_AXI_awuser(versal_cips_0_LPD_AXI_NOC_0_AWUSER),
        .S04_AXI_awvalid(versal_cips_0_LPD_AXI_NOC_0_AWVALID),
        .S04_AXI_bid(versal_cips_0_LPD_AXI_NOC_0_BID),
        .S04_AXI_bready(versal_cips_0_LPD_AXI_NOC_0_BREADY),
        .S04_AXI_bresp(versal_cips_0_LPD_AXI_NOC_0_BRESP),
        .S04_AXI_bvalid(versal_cips_0_LPD_AXI_NOC_0_BVALID),
        .S04_AXI_rdata(versal_cips_0_LPD_AXI_NOC_0_RDATA),
        .S04_AXI_rid(versal_cips_0_LPD_AXI_NOC_0_RID),
        .S04_AXI_rlast(versal_cips_0_LPD_AXI_NOC_0_RLAST),
        .S04_AXI_rready(versal_cips_0_LPD_AXI_NOC_0_RREADY),
        .S04_AXI_rresp(versal_cips_0_LPD_AXI_NOC_0_RRESP),
        .S04_AXI_rvalid(versal_cips_0_LPD_AXI_NOC_0_RVALID),
        .S04_AXI_wdata(versal_cips_0_LPD_AXI_NOC_0_WDATA),
        .S04_AXI_wlast(versal_cips_0_LPD_AXI_NOC_0_WLAST),
        .S04_AXI_wready(versal_cips_0_LPD_AXI_NOC_0_WREADY),
        .S04_AXI_wstrb(versal_cips_0_LPD_AXI_NOC_0_WSTRB),
        .S04_AXI_wuser({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S04_AXI_wvalid(versal_cips_0_LPD_AXI_NOC_0_WVALID),
        .S05_AXI_araddr(versal_cips_0_PMC_NOC_AXI_0_ARADDR),
        .S05_AXI_arburst(versal_cips_0_PMC_NOC_AXI_0_ARBURST),
        .S05_AXI_arcache(versal_cips_0_PMC_NOC_AXI_0_ARCACHE),
        .S05_AXI_arid(versal_cips_0_PMC_NOC_AXI_0_ARID),
        .S05_AXI_arlen(versal_cips_0_PMC_NOC_AXI_0_ARLEN),
        .S05_AXI_arlock(versal_cips_0_PMC_NOC_AXI_0_ARLOCK),
        .S05_AXI_arprot(versal_cips_0_PMC_NOC_AXI_0_ARPROT),
        .S05_AXI_arqos(versal_cips_0_PMC_NOC_AXI_0_ARQOS),
        .S05_AXI_arready(versal_cips_0_PMC_NOC_AXI_0_ARREADY),
        .S05_AXI_arregion(versal_cips_0_PMC_NOC_AXI_0_ARREGION),
        .S05_AXI_arsize(versal_cips_0_PMC_NOC_AXI_0_ARSIZE),
        .S05_AXI_aruser(versal_cips_0_PMC_NOC_AXI_0_ARUSER),
        .S05_AXI_arvalid(versal_cips_0_PMC_NOC_AXI_0_ARVALID),
        .S05_AXI_awaddr(versal_cips_0_PMC_NOC_AXI_0_AWADDR),
        .S05_AXI_awburst(versal_cips_0_PMC_NOC_AXI_0_AWBURST),
        .S05_AXI_awcache(versal_cips_0_PMC_NOC_AXI_0_AWCACHE),
        .S05_AXI_awid(versal_cips_0_PMC_NOC_AXI_0_AWID),
        .S05_AXI_awlen(versal_cips_0_PMC_NOC_AXI_0_AWLEN),
        .S05_AXI_awlock(versal_cips_0_PMC_NOC_AXI_0_AWLOCK),
        .S05_AXI_awprot(versal_cips_0_PMC_NOC_AXI_0_AWPROT),
        .S05_AXI_awqos(versal_cips_0_PMC_NOC_AXI_0_AWQOS),
        .S05_AXI_awready(versal_cips_0_PMC_NOC_AXI_0_AWREADY),
        .S05_AXI_awregion(versal_cips_0_PMC_NOC_AXI_0_AWREGION),
        .S05_AXI_awsize(versal_cips_0_PMC_NOC_AXI_0_AWSIZE),
        .S05_AXI_awuser(versal_cips_0_PMC_NOC_AXI_0_AWUSER),
        .S05_AXI_awvalid(versal_cips_0_PMC_NOC_AXI_0_AWVALID),
        .S05_AXI_bid(versal_cips_0_PMC_NOC_AXI_0_BID),
        .S05_AXI_bready(versal_cips_0_PMC_NOC_AXI_0_BREADY),
        .S05_AXI_bresp(versal_cips_0_PMC_NOC_AXI_0_BRESP),
        .S05_AXI_buser(versal_cips_0_PMC_NOC_AXI_0_BUSER),
        .S05_AXI_bvalid(versal_cips_0_PMC_NOC_AXI_0_BVALID),
        .S05_AXI_rdata(versal_cips_0_PMC_NOC_AXI_0_RDATA),
        .S05_AXI_rid(versal_cips_0_PMC_NOC_AXI_0_RID),
        .S05_AXI_rlast(versal_cips_0_PMC_NOC_AXI_0_RLAST),
        .S05_AXI_rready(versal_cips_0_PMC_NOC_AXI_0_RREADY),
        .S05_AXI_rresp(versal_cips_0_PMC_NOC_AXI_0_RRESP),
        .S05_AXI_ruser(versal_cips_0_PMC_NOC_AXI_0_RUSER),
        .S05_AXI_rvalid(versal_cips_0_PMC_NOC_AXI_0_RVALID),
        .S05_AXI_wdata(versal_cips_0_PMC_NOC_AXI_0_WDATA),
        .S05_AXI_wlast(versal_cips_0_PMC_NOC_AXI_0_WLAST),
        .S05_AXI_wready(versal_cips_0_PMC_NOC_AXI_0_WREADY),
        .S05_AXI_wstrb(versal_cips_0_PMC_NOC_AXI_0_WSTRB),
        .S05_AXI_wuser(versal_cips_0_PMC_NOC_AXI_0_WUSER),
        .S05_AXI_wvalid(versal_cips_0_PMC_NOC_AXI_0_WVALID),
        .S06_AXI_araddr(axi_dma_0_M_AXI_MM2S_ARADDR),
        .S06_AXI_arburst(axi_dma_0_M_AXI_MM2S_ARBURST),
        .S06_AXI_arcache(axi_dma_0_M_AXI_MM2S_ARCACHE),
        .S06_AXI_arlen(axi_dma_0_M_AXI_MM2S_ARLEN),
        .S06_AXI_arlock(1'b0),
        .S06_AXI_arprot(axi_dma_0_M_AXI_MM2S_ARPROT),
        .S06_AXI_arqos({1'b0,1'b0,1'b0,1'b0}),
        .S06_AXI_arready(axi_dma_0_M_AXI_MM2S_ARREADY),
        .S06_AXI_arregion({1'b0,1'b0,1'b0,1'b0}),
        .S06_AXI_arsize(axi_dma_0_M_AXI_MM2S_ARSIZE),
        .S06_AXI_arvalid(axi_dma_0_M_AXI_MM2S_ARVALID),
        .S06_AXI_awaddr({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S06_AXI_awburst({1'b0,1'b1}),
        .S06_AXI_awcache({1'b0,1'b0,1'b1,1'b1}),
        .S06_AXI_awlen({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S06_AXI_awlock(1'b0),
        .S06_AXI_awprot({1'b0,1'b0,1'b0}),
        .S06_AXI_awqos({1'b0,1'b0,1'b0,1'b0}),
        .S06_AXI_awregion({1'b0,1'b0,1'b0,1'b0}),
        .S06_AXI_awsize({1'b0,1'b1,1'b1}),
        .S06_AXI_awvalid(1'b0),
        .S06_AXI_bready(1'b0),
        .S06_AXI_rdata(axi_dma_0_M_AXI_MM2S_RDATA),
        .S06_AXI_rlast(axi_dma_0_M_AXI_MM2S_RLAST),
        .S06_AXI_rready(axi_dma_0_M_AXI_MM2S_RREADY),
        .S06_AXI_rresp(axi_dma_0_M_AXI_MM2S_RRESP),
        .S06_AXI_rvalid(axi_dma_0_M_AXI_MM2S_RVALID),
        .S06_AXI_wdata({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S06_AXI_wlast(1'b0),
        .S06_AXI_wstrb({1'b1,1'b1,1'b1,1'b1,1'b1,1'b1,1'b1,1'b1}),
        .S06_AXI_wvalid(1'b0),
        .S07_AXI_araddr(covariance_estimation_0_m_axi_gmem_ARADDR),
        .S07_AXI_arburst(covariance_estimation_0_m_axi_gmem_ARBURST),
        .S07_AXI_arcache(covariance_estimation_0_m_axi_gmem_ARCACHE),
        .S07_AXI_arid(covariance_estimation_0_m_axi_gmem_ARID),
        .S07_AXI_arlen(covariance_estimation_0_m_axi_gmem_ARLEN),
        .S07_AXI_arlock(covariance_estimation_0_m_axi_gmem_ARLOCK[0]),
        .S07_AXI_arprot(covariance_estimation_0_m_axi_gmem_ARPROT),
        .S07_AXI_arqos(covariance_estimation_0_m_axi_gmem_ARQOS),
        .S07_AXI_arready(covariance_estimation_0_m_axi_gmem_ARREADY),
        .S07_AXI_arregion(covariance_estimation_0_m_axi_gmem_ARREGION),
        .S07_AXI_arsize(covariance_estimation_0_m_axi_gmem_ARSIZE),
        .S07_AXI_arvalid(covariance_estimation_0_m_axi_gmem_ARVALID),
        .S07_AXI_awaddr(covariance_estimation_0_m_axi_gmem_AWADDR),
        .S07_AXI_awburst(covariance_estimation_0_m_axi_gmem_AWBURST),
        .S07_AXI_awcache(covariance_estimation_0_m_axi_gmem_AWCACHE),
        .S07_AXI_awid(covariance_estimation_0_m_axi_gmem_AWID),
        .S07_AXI_awlen(covariance_estimation_0_m_axi_gmem_AWLEN),
        .S07_AXI_awlock(covariance_estimation_0_m_axi_gmem_AWLOCK[0]),
        .S07_AXI_awprot(covariance_estimation_0_m_axi_gmem_AWPROT),
        .S07_AXI_awqos(covariance_estimation_0_m_axi_gmem_AWQOS),
        .S07_AXI_awready(covariance_estimation_0_m_axi_gmem_AWREADY),
        .S07_AXI_awregion(covariance_estimation_0_m_axi_gmem_AWREGION),
        .S07_AXI_awsize(covariance_estimation_0_m_axi_gmem_AWSIZE),
        .S07_AXI_awvalid(covariance_estimation_0_m_axi_gmem_AWVALID),
        .S07_AXI_bid(covariance_estimation_0_m_axi_gmem_BID),
        .S07_AXI_bready(covariance_estimation_0_m_axi_gmem_BREADY),
        .S07_AXI_bresp(covariance_estimation_0_m_axi_gmem_BRESP),
        .S07_AXI_bvalid(covariance_estimation_0_m_axi_gmem_BVALID),
        .S07_AXI_rdata(covariance_estimation_0_m_axi_gmem_RDATA),
        .S07_AXI_rid(covariance_estimation_0_m_axi_gmem_RID),
        .S07_AXI_rlast(covariance_estimation_0_m_axi_gmem_RLAST),
        .S07_AXI_rready(covariance_estimation_0_m_axi_gmem_RREADY),
        .S07_AXI_rresp(covariance_estimation_0_m_axi_gmem_RRESP),
        .S07_AXI_rvalid(covariance_estimation_0_m_axi_gmem_RVALID),
        .S07_AXI_wdata(covariance_estimation_0_m_axi_gmem_WDATA),
        .S07_AXI_wlast(covariance_estimation_0_m_axi_gmem_WLAST),
        .S07_AXI_wready(covariance_estimation_0_m_axi_gmem_WREADY),
        .S07_AXI_wstrb(covariance_estimation_0_m_axi_gmem_WSTRB),
        .S07_AXI_wvalid(covariance_estimation_0_m_axi_gmem_WVALID),
        .S08_AXI_araddr(smartconnect_mem_0_M00_AXI_ARADDR),
        .S08_AXI_arburst(smartconnect_mem_0_M00_AXI_ARBURST),
        .S08_AXI_arcache(smartconnect_mem_0_M00_AXI_ARCACHE),
        .S08_AXI_arlen(smartconnect_mem_0_M00_AXI_ARLEN),
        .S08_AXI_arlock(smartconnect_mem_0_M00_AXI_ARLOCK),
        .S08_AXI_arprot(smartconnect_mem_0_M00_AXI_ARPROT),
        .S08_AXI_arqos(smartconnect_mem_0_M00_AXI_ARQOS),
        .S08_AXI_arready(smartconnect_mem_0_M00_AXI_ARREADY),
        .S08_AXI_arregion({1'b0,1'b0,1'b0,1'b0}),
        .S08_AXI_arsize(smartconnect_mem_0_M00_AXI_ARSIZE),
        .S08_AXI_arvalid(smartconnect_mem_0_M00_AXI_ARVALID),
        .S08_AXI_awaddr(smartconnect_mem_0_M00_AXI_AWADDR),
        .S08_AXI_awburst(smartconnect_mem_0_M00_AXI_AWBURST),
        .S08_AXI_awcache(smartconnect_mem_0_M00_AXI_AWCACHE),
        .S08_AXI_awlen(smartconnect_mem_0_M00_AXI_AWLEN),
        .S08_AXI_awlock(smartconnect_mem_0_M00_AXI_AWLOCK),
        .S08_AXI_awprot(smartconnect_mem_0_M00_AXI_AWPROT),
        .S08_AXI_awqos(smartconnect_mem_0_M00_AXI_AWQOS),
        .S08_AXI_awready(smartconnect_mem_0_M00_AXI_AWREADY),
        .S08_AXI_awregion({1'b0,1'b0,1'b0,1'b0}),
        .S08_AXI_awsize(smartconnect_mem_0_M00_AXI_AWSIZE),
        .S08_AXI_awvalid(smartconnect_mem_0_M00_AXI_AWVALID),
        .S08_AXI_bready(smartconnect_mem_0_M00_AXI_BREADY),
        .S08_AXI_bresp(smartconnect_mem_0_M00_AXI_BRESP),
        .S08_AXI_bvalid(smartconnect_mem_0_M00_AXI_BVALID),
        .S08_AXI_rdata(smartconnect_mem_0_M00_AXI_RDATA),
        .S08_AXI_rlast(smartconnect_mem_0_M00_AXI_RLAST),
        .S08_AXI_rready(smartconnect_mem_0_M00_AXI_RREADY),
        .S08_AXI_rresp(smartconnect_mem_0_M00_AXI_RRESP),
        .S08_AXI_rvalid(smartconnect_mem_0_M00_AXI_RVALID),
        .S08_AXI_wdata(smartconnect_mem_0_M00_AXI_WDATA),
        .S08_AXI_wlast(smartconnect_mem_0_M00_AXI_WLAST),
        .S08_AXI_wready(smartconnect_mem_0_M00_AXI_WREADY),
        .S08_AXI_wstrb(smartconnect_mem_0_M00_AXI_WSTRB),
        .S08_AXI_wvalid(smartconnect_mem_0_M00_AXI_WVALID),
        .aclk0(versal_cips_0_fpd_cci_noc_axi0_clk),
        .aclk1(versal_cips_0_fpd_cci_noc_axi1_clk),
        .aclk2(versal_cips_0_fpd_cci_noc_axi2_clk),
        .aclk3(versal_cips_0_fpd_cci_noc_axi3_clk),
        .aclk4(versal_cips_0_lpd_axi_noc_clk),
        .aclk5(versal_cips_0_pmc_axi_noc_axi0_clk),
        .aclk6(clk_wizard_0_clk_out1),
        .sys_clk0_clk_n(ddr4_dimm1_sma_clk_clk_n),
        .sys_clk0_clk_p(ddr4_dimm1_sma_clk_clk_p));
  mvdrr_clk_wizard_0_0 clk_wizard_0
       (.clk_in1(versal_cips_0_pl0_ref_clk),
        .clk_out1(clk_wizard_0_clk_out1),
        .locked(clk_wizard_0_locked),
        .resetn(versal_cips_0_pl0_resetn));
  mvdrr_covariance_estimation_0_0 covariance_estimation_0
       (.ap_clk(clk_wizard_0_clk_out1),
        .ap_rst_n(proc_sys_reset_0_peripheral_aresetn),
        .interrupt(covariance_estimation_0_interrupt),
        .m_axi_gmem_ARADDR(covariance_estimation_0_m_axi_gmem_ARADDR),
        .m_axi_gmem_ARBURST(covariance_estimation_0_m_axi_gmem_ARBURST),
        .m_axi_gmem_ARCACHE(covariance_estimation_0_m_axi_gmem_ARCACHE),
        .m_axi_gmem_ARID(covariance_estimation_0_m_axi_gmem_ARID),
        .m_axi_gmem_ARLEN(covariance_estimation_0_m_axi_gmem_ARLEN),
        .m_axi_gmem_ARLOCK(covariance_estimation_0_m_axi_gmem_ARLOCK),
        .m_axi_gmem_ARPROT(covariance_estimation_0_m_axi_gmem_ARPROT),
        .m_axi_gmem_ARQOS(covariance_estimation_0_m_axi_gmem_ARQOS),
        .m_axi_gmem_ARREADY(covariance_estimation_0_m_axi_gmem_ARREADY),
        .m_axi_gmem_ARREGION(covariance_estimation_0_m_axi_gmem_ARREGION),
        .m_axi_gmem_ARSIZE(covariance_estimation_0_m_axi_gmem_ARSIZE),
        .m_axi_gmem_ARVALID(covariance_estimation_0_m_axi_gmem_ARVALID),
        .m_axi_gmem_AWADDR(covariance_estimation_0_m_axi_gmem_AWADDR),
        .m_axi_gmem_AWBURST(covariance_estimation_0_m_axi_gmem_AWBURST),
        .m_axi_gmem_AWCACHE(covariance_estimation_0_m_axi_gmem_AWCACHE),
        .m_axi_gmem_AWID(covariance_estimation_0_m_axi_gmem_AWID),
        .m_axi_gmem_AWLEN(covariance_estimation_0_m_axi_gmem_AWLEN),
        .m_axi_gmem_AWLOCK(covariance_estimation_0_m_axi_gmem_AWLOCK),
        .m_axi_gmem_AWPROT(covariance_estimation_0_m_axi_gmem_AWPROT),
        .m_axi_gmem_AWQOS(covariance_estimation_0_m_axi_gmem_AWQOS),
        .m_axi_gmem_AWREADY(covariance_estimation_0_m_axi_gmem_AWREADY),
        .m_axi_gmem_AWREGION(covariance_estimation_0_m_axi_gmem_AWREGION),
        .m_axi_gmem_AWSIZE(covariance_estimation_0_m_axi_gmem_AWSIZE),
        .m_axi_gmem_AWVALID(covariance_estimation_0_m_axi_gmem_AWVALID),
        .m_axi_gmem_BID(covariance_estimation_0_m_axi_gmem_BID),
        .m_axi_gmem_BREADY(covariance_estimation_0_m_axi_gmem_BREADY),
        .m_axi_gmem_BRESP(covariance_estimation_0_m_axi_gmem_BRESP),
        .m_axi_gmem_BVALID(covariance_estimation_0_m_axi_gmem_BVALID),
        .m_axi_gmem_RDATA(covariance_estimation_0_m_axi_gmem_RDATA),
        .m_axi_gmem_RID(covariance_estimation_0_m_axi_gmem_RID),
        .m_axi_gmem_RLAST(covariance_estimation_0_m_axi_gmem_RLAST),
        .m_axi_gmem_RREADY(covariance_estimation_0_m_axi_gmem_RREADY),
        .m_axi_gmem_RRESP(covariance_estimation_0_m_axi_gmem_RRESP),
        .m_axi_gmem_RVALID(covariance_estimation_0_m_axi_gmem_RVALID),
        .m_axi_gmem_WDATA(covariance_estimation_0_m_axi_gmem_WDATA),
        .m_axi_gmem_WLAST(covariance_estimation_0_m_axi_gmem_WLAST),
        .m_axi_gmem_WREADY(covariance_estimation_0_m_axi_gmem_WREADY),
        .m_axi_gmem_WSTRB(covariance_estimation_0_m_axi_gmem_WSTRB),
        .m_axi_gmem_WVALID(covariance_estimation_0_m_axi_gmem_WVALID),
        .s_axi_ctrl_ARADDR(smartconnect_0_M01_AXI_ARADDR),
        .s_axi_ctrl_ARREADY(smartconnect_0_M01_AXI_ARREADY),
        .s_axi_ctrl_ARVALID(smartconnect_0_M01_AXI_ARVALID),
        .s_axi_ctrl_AWADDR(smartconnect_0_M01_AXI_AWADDR),
        .s_axi_ctrl_AWREADY(smartconnect_0_M01_AXI_AWREADY),
        .s_axi_ctrl_AWVALID(smartconnect_0_M01_AXI_AWVALID),
        .s_axi_ctrl_BREADY(smartconnect_0_M01_AXI_BREADY),
        .s_axi_ctrl_BRESP(smartconnect_0_M01_AXI_BRESP),
        .s_axi_ctrl_BVALID(smartconnect_0_M01_AXI_BVALID),
        .s_axi_ctrl_RDATA(smartconnect_0_M01_AXI_RDATA),
        .s_axi_ctrl_RREADY(smartconnect_0_M01_AXI_RREADY),
        .s_axi_ctrl_RRESP(smartconnect_0_M01_AXI_RRESP),
        .s_axi_ctrl_RVALID(smartconnect_0_M01_AXI_RVALID),
        .s_axi_ctrl_WDATA(smartconnect_0_M01_AXI_WDATA),
        .s_axi_ctrl_WREADY(smartconnect_0_M01_AXI_WREADY),
        .s_axi_ctrl_WSTRB(smartconnect_0_M01_AXI_WSTRB),
        .s_axi_ctrl_WVALID(smartconnect_0_M01_AXI_WVALID),
        .y_in_TDATA(rdbs_kernel_0_y_out_TDATA),
        .y_in_TREADY(rdbs_kernel_0_y_out_TREADY),
        .y_in_TVALID(rdbs_kernel_0_y_out_TVALID));
  mvdrr_mvdr_weights_0_0 mvdr_weights_0
       (.ap_clk(clk_wizard_0_clk_out1),
        .ap_rst_n(proc_sys_reset_0_peripheral_aresetn),
        .interrupt(mvdr_weights_0_interrupt),
        .m_axi_gmem0_ARADDR(mvdr_weights_0_m_axi_gmem0_ARADDR),
        .m_axi_gmem0_ARBURST(mvdr_weights_0_m_axi_gmem0_ARBURST),
        .m_axi_gmem0_ARCACHE(mvdr_weights_0_m_axi_gmem0_ARCACHE),
        .m_axi_gmem0_ARID(mvdr_weights_0_m_axi_gmem0_ARID),
        .m_axi_gmem0_ARLEN(mvdr_weights_0_m_axi_gmem0_ARLEN),
        .m_axi_gmem0_ARLOCK(mvdr_weights_0_m_axi_gmem0_ARLOCK),
        .m_axi_gmem0_ARPROT(mvdr_weights_0_m_axi_gmem0_ARPROT),
        .m_axi_gmem0_ARQOS(mvdr_weights_0_m_axi_gmem0_ARQOS),
        .m_axi_gmem0_ARREADY(mvdr_weights_0_m_axi_gmem0_ARREADY),
        .m_axi_gmem0_ARSIZE(mvdr_weights_0_m_axi_gmem0_ARSIZE),
        .m_axi_gmem0_ARVALID(mvdr_weights_0_m_axi_gmem0_ARVALID),
        .m_axi_gmem0_AWREADY(1'b0),
        .m_axi_gmem0_BID(1'b0),
        .m_axi_gmem0_BRESP({1'b0,1'b0}),
        .m_axi_gmem0_BVALID(1'b0),
        .m_axi_gmem0_RDATA(mvdr_weights_0_m_axi_gmem0_RDATA),
        .m_axi_gmem0_RID(mvdr_weights_0_m_axi_gmem0_RID),
        .m_axi_gmem0_RLAST(mvdr_weights_0_m_axi_gmem0_RLAST),
        .m_axi_gmem0_RREADY(mvdr_weights_0_m_axi_gmem0_RREADY),
        .m_axi_gmem0_RRESP(mvdr_weights_0_m_axi_gmem0_RRESP),
        .m_axi_gmem0_RVALID(mvdr_weights_0_m_axi_gmem0_RVALID),
        .m_axi_gmem0_WREADY(1'b0),
        .m_axi_gmem1_ARADDR(mvdr_weights_0_m_axi_gmem1_ARADDR),
        .m_axi_gmem1_ARBURST(mvdr_weights_0_m_axi_gmem1_ARBURST),
        .m_axi_gmem1_ARCACHE(mvdr_weights_0_m_axi_gmem1_ARCACHE),
        .m_axi_gmem1_ARID(mvdr_weights_0_m_axi_gmem1_ARID),
        .m_axi_gmem1_ARLEN(mvdr_weights_0_m_axi_gmem1_ARLEN),
        .m_axi_gmem1_ARLOCK(mvdr_weights_0_m_axi_gmem1_ARLOCK),
        .m_axi_gmem1_ARPROT(mvdr_weights_0_m_axi_gmem1_ARPROT),
        .m_axi_gmem1_ARQOS(mvdr_weights_0_m_axi_gmem1_ARQOS),
        .m_axi_gmem1_ARREADY(mvdr_weights_0_m_axi_gmem1_ARREADY),
        .m_axi_gmem1_ARSIZE(mvdr_weights_0_m_axi_gmem1_ARSIZE),
        .m_axi_gmem1_ARVALID(mvdr_weights_0_m_axi_gmem1_ARVALID),
        .m_axi_gmem1_AWREADY(1'b0),
        .m_axi_gmem1_BID(1'b0),
        .m_axi_gmem1_BRESP({1'b0,1'b0}),
        .m_axi_gmem1_BVALID(1'b0),
        .m_axi_gmem1_RDATA(mvdr_weights_0_m_axi_gmem1_RDATA),
        .m_axi_gmem1_RID(mvdr_weights_0_m_axi_gmem1_RID),
        .m_axi_gmem1_RLAST(mvdr_weights_0_m_axi_gmem1_RLAST),
        .m_axi_gmem1_RREADY(mvdr_weights_0_m_axi_gmem1_RREADY),
        .m_axi_gmem1_RRESP(mvdr_weights_0_m_axi_gmem1_RRESP),
        .m_axi_gmem1_RVALID(mvdr_weights_0_m_axi_gmem1_RVALID),
        .m_axi_gmem1_WREADY(1'b0),
        .m_axi_gmem2_ARREADY(1'b0),
        .m_axi_gmem2_AWADDR(mvdr_weights_0_m_axi_gmem2_AWADDR),
        .m_axi_gmem2_AWBURST(mvdr_weights_0_m_axi_gmem2_AWBURST),
        .m_axi_gmem2_AWCACHE(mvdr_weights_0_m_axi_gmem2_AWCACHE),
        .m_axi_gmem2_AWID(mvdr_weights_0_m_axi_gmem2_AWID),
        .m_axi_gmem2_AWLEN(mvdr_weights_0_m_axi_gmem2_AWLEN),
        .m_axi_gmem2_AWLOCK(mvdr_weights_0_m_axi_gmem2_AWLOCK),
        .m_axi_gmem2_AWPROT(mvdr_weights_0_m_axi_gmem2_AWPROT),
        .m_axi_gmem2_AWQOS(mvdr_weights_0_m_axi_gmem2_AWQOS),
        .m_axi_gmem2_AWREADY(mvdr_weights_0_m_axi_gmem2_AWREADY),
        .m_axi_gmem2_AWSIZE(mvdr_weights_0_m_axi_gmem2_AWSIZE),
        .m_axi_gmem2_AWVALID(mvdr_weights_0_m_axi_gmem2_AWVALID),
        .m_axi_gmem2_BID(mvdr_weights_0_m_axi_gmem2_BID),
        .m_axi_gmem2_BREADY(mvdr_weights_0_m_axi_gmem2_BREADY),
        .m_axi_gmem2_BRESP(mvdr_weights_0_m_axi_gmem2_BRESP),
        .m_axi_gmem2_BVALID(mvdr_weights_0_m_axi_gmem2_BVALID),
        .m_axi_gmem2_RDATA({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .m_axi_gmem2_RID(1'b0),
        .m_axi_gmem2_RLAST(1'b0),
        .m_axi_gmem2_RRESP({1'b0,1'b0}),
        .m_axi_gmem2_RVALID(1'b0),
        .m_axi_gmem2_WDATA(mvdr_weights_0_m_axi_gmem2_WDATA),
        .m_axi_gmem2_WLAST(mvdr_weights_0_m_axi_gmem2_WLAST),
        .m_axi_gmem2_WREADY(mvdr_weights_0_m_axi_gmem2_WREADY),
        .m_axi_gmem2_WSTRB(mvdr_weights_0_m_axi_gmem2_WSTRB),
        .m_axi_gmem2_WVALID(mvdr_weights_0_m_axi_gmem2_WVALID),
        .s_axi_ctrl_ARADDR(smartconnect_0_M02_AXI_ARADDR),
        .s_axi_ctrl_ARREADY(smartconnect_0_M02_AXI_ARREADY),
        .s_axi_ctrl_ARVALID(smartconnect_0_M02_AXI_ARVALID),
        .s_axi_ctrl_AWADDR(smartconnect_0_M02_AXI_AWADDR),
        .s_axi_ctrl_AWREADY(smartconnect_0_M02_AXI_AWREADY),
        .s_axi_ctrl_AWVALID(smartconnect_0_M02_AXI_AWVALID),
        .s_axi_ctrl_BREADY(smartconnect_0_M02_AXI_BREADY),
        .s_axi_ctrl_BRESP(smartconnect_0_M02_AXI_BRESP),
        .s_axi_ctrl_BVALID(smartconnect_0_M02_AXI_BVALID),
        .s_axi_ctrl_RDATA(smartconnect_0_M02_AXI_RDATA),
        .s_axi_ctrl_RREADY(smartconnect_0_M02_AXI_RREADY),
        .s_axi_ctrl_RRESP(smartconnect_0_M02_AXI_RRESP),
        .s_axi_ctrl_RVALID(smartconnect_0_M02_AXI_RVALID),
        .s_axi_ctrl_WDATA(smartconnect_0_M02_AXI_WDATA),
        .s_axi_ctrl_WREADY(smartconnect_0_M02_AXI_WREADY),
        .s_axi_ctrl_WSTRB(smartconnect_0_M02_AXI_WSTRB),
        .s_axi_ctrl_WVALID(smartconnect_0_M02_AXI_WVALID));
  mvdrr_proc_sys_reset_0_0 proc_sys_reset_0
       (.aux_reset_in(1'b1),
        .dcm_locked(clk_wizard_0_locked),
        .ext_reset_in(versal_cips_0_pl0_resetn),
        .mb_debug_sys_rst(1'b0),
        .peripheral_aresetn(proc_sys_reset_0_peripheral_aresetn),
        .slowest_sync_clk(clk_wizard_0_clk_out1));
  mvdrr_rdbs_kernel_0_0 rdbs_kernel_0
       (.ap_clk(clk_wizard_0_clk_out1),
        .ap_rst_n(proc_sys_reset_0_peripheral_aresetn),
        .interrupt(rdbs_kernel_0_interrupt),
        .s_axi_control_ARADDR(smartconnect_0_M00_AXI_ARADDR),
        .s_axi_control_ARREADY(smartconnect_0_M00_AXI_ARREADY),
        .s_axi_control_ARVALID(smartconnect_0_M00_AXI_ARVALID),
        .s_axi_control_AWADDR(smartconnect_0_M00_AXI_AWADDR),
        .s_axi_control_AWREADY(smartconnect_0_M00_AXI_AWREADY),
        .s_axi_control_AWVALID(smartconnect_0_M00_AXI_AWVALID),
        .s_axi_control_BREADY(smartconnect_0_M00_AXI_BREADY),
        .s_axi_control_BRESP(smartconnect_0_M00_AXI_BRESP),
        .s_axi_control_BVALID(smartconnect_0_M00_AXI_BVALID),
        .s_axi_control_RDATA(smartconnect_0_M00_AXI_RDATA),
        .s_axi_control_RREADY(smartconnect_0_M00_AXI_RREADY),
        .s_axi_control_RRESP(smartconnect_0_M00_AXI_RRESP),
        .s_axi_control_RVALID(smartconnect_0_M00_AXI_RVALID),
        .s_axi_control_WDATA(smartconnect_0_M00_AXI_WDATA),
        .s_axi_control_WREADY(smartconnect_0_M00_AXI_WREADY),
        .s_axi_control_WSTRB(smartconnect_0_M00_AXI_WSTRB),
        .s_axi_control_WVALID(smartconnect_0_M00_AXI_WVALID),
        .x_in_TDATA(axi_dma_0_M_AXIS_MM2S_TDATA),
        .x_in_TREADY(axi_dma_0_M_AXIS_MM2S_TREADY),
        .x_in_TVALID(axi_dma_0_M_AXIS_MM2S_TVALID),
        .y_out_TDATA(rdbs_kernel_0_y_out_TDATA),
        .y_out_TREADY(rdbs_kernel_0_y_out_TREADY),
        .y_out_TVALID(rdbs_kernel_0_y_out_TVALID));
  mvdrr_smartconnect_0_0 smartconnect_0
       (.M00_AXI_araddr(smartconnect_0_M00_AXI_ARADDR),
        .M00_AXI_arready(smartconnect_0_M00_AXI_ARREADY),
        .M00_AXI_arvalid(smartconnect_0_M00_AXI_ARVALID),
        .M00_AXI_awaddr(smartconnect_0_M00_AXI_AWADDR),
        .M00_AXI_awready(smartconnect_0_M00_AXI_AWREADY),
        .M00_AXI_awvalid(smartconnect_0_M00_AXI_AWVALID),
        .M00_AXI_bready(smartconnect_0_M00_AXI_BREADY),
        .M00_AXI_bresp(smartconnect_0_M00_AXI_BRESP),
        .M00_AXI_bvalid(smartconnect_0_M00_AXI_BVALID),
        .M00_AXI_rdata(smartconnect_0_M00_AXI_RDATA),
        .M00_AXI_rready(smartconnect_0_M00_AXI_RREADY),
        .M00_AXI_rresp(smartconnect_0_M00_AXI_RRESP),
        .M00_AXI_rvalid(smartconnect_0_M00_AXI_RVALID),
        .M00_AXI_wdata(smartconnect_0_M00_AXI_WDATA),
        .M00_AXI_wready(smartconnect_0_M00_AXI_WREADY),
        .M00_AXI_wstrb(smartconnect_0_M00_AXI_WSTRB),
        .M00_AXI_wvalid(smartconnect_0_M00_AXI_WVALID),
        .M01_AXI_araddr(smartconnect_0_M01_AXI_ARADDR),
        .M01_AXI_arready(smartconnect_0_M01_AXI_ARREADY),
        .M01_AXI_arvalid(smartconnect_0_M01_AXI_ARVALID),
        .M01_AXI_awaddr(smartconnect_0_M01_AXI_AWADDR),
        .M01_AXI_awready(smartconnect_0_M01_AXI_AWREADY),
        .M01_AXI_awvalid(smartconnect_0_M01_AXI_AWVALID),
        .M01_AXI_bready(smartconnect_0_M01_AXI_BREADY),
        .M01_AXI_bresp(smartconnect_0_M01_AXI_BRESP),
        .M01_AXI_bvalid(smartconnect_0_M01_AXI_BVALID),
        .M01_AXI_rdata(smartconnect_0_M01_AXI_RDATA),
        .M01_AXI_rready(smartconnect_0_M01_AXI_RREADY),
        .M01_AXI_rresp(smartconnect_0_M01_AXI_RRESP),
        .M01_AXI_rvalid(smartconnect_0_M01_AXI_RVALID),
        .M01_AXI_wdata(smartconnect_0_M01_AXI_WDATA),
        .M01_AXI_wready(smartconnect_0_M01_AXI_WREADY),
        .M01_AXI_wstrb(smartconnect_0_M01_AXI_WSTRB),
        .M01_AXI_wvalid(smartconnect_0_M01_AXI_WVALID),
        .M02_AXI_araddr(smartconnect_0_M02_AXI_ARADDR),
        .M02_AXI_arready(smartconnect_0_M02_AXI_ARREADY),
        .M02_AXI_arvalid(smartconnect_0_M02_AXI_ARVALID),
        .M02_AXI_awaddr(smartconnect_0_M02_AXI_AWADDR),
        .M02_AXI_awready(smartconnect_0_M02_AXI_AWREADY),
        .M02_AXI_awvalid(smartconnect_0_M02_AXI_AWVALID),
        .M02_AXI_bready(smartconnect_0_M02_AXI_BREADY),
        .M02_AXI_bresp(smartconnect_0_M02_AXI_BRESP),
        .M02_AXI_bvalid(smartconnect_0_M02_AXI_BVALID),
        .M02_AXI_rdata(smartconnect_0_M02_AXI_RDATA),
        .M02_AXI_rready(smartconnect_0_M02_AXI_RREADY),
        .M02_AXI_rresp(smartconnect_0_M02_AXI_RRESP),
        .M02_AXI_rvalid(smartconnect_0_M02_AXI_RVALID),
        .M02_AXI_wdata(smartconnect_0_M02_AXI_WDATA),
        .M02_AXI_wready(smartconnect_0_M02_AXI_WREADY),
        .M02_AXI_wstrb(smartconnect_0_M02_AXI_WSTRB),
        .M02_AXI_wvalid(smartconnect_0_M02_AXI_WVALID),
        .M03_AXI_araddr(smartconnect_0_M03_AXI_ARADDR),
        .M03_AXI_arready(smartconnect_0_M03_AXI_ARREADY),
        .M03_AXI_arvalid(smartconnect_0_M03_AXI_ARVALID),
        .M03_AXI_awaddr(smartconnect_0_M03_AXI_AWADDR),
        .M03_AXI_awready(smartconnect_0_M03_AXI_AWREADY),
        .M03_AXI_awvalid(smartconnect_0_M03_AXI_AWVALID),
        .M03_AXI_bready(smartconnect_0_M03_AXI_BREADY),
        .M03_AXI_bresp(smartconnect_0_M03_AXI_BRESP),
        .M03_AXI_bvalid(smartconnect_0_M03_AXI_BVALID),
        .M03_AXI_rdata(smartconnect_0_M03_AXI_RDATA),
        .M03_AXI_rready(smartconnect_0_M03_AXI_RREADY),
        .M03_AXI_rresp(smartconnect_0_M03_AXI_RRESP),
        .M03_AXI_rvalid(smartconnect_0_M03_AXI_RVALID),
        .M03_AXI_wdata(smartconnect_0_M03_AXI_WDATA),
        .M03_AXI_wready(smartconnect_0_M03_AXI_WREADY),
        .M03_AXI_wvalid(smartconnect_0_M03_AXI_WVALID),
        .S00_AXI_araddr(versal_cips_0_M_AXI_FPD_ARADDR),
        .S00_AXI_arburst(versal_cips_0_M_AXI_FPD_ARBURST),
        .S00_AXI_arcache(versal_cips_0_M_AXI_FPD_ARCACHE),
        .S00_AXI_arid(versal_cips_0_M_AXI_FPD_ARID),
        .S00_AXI_arlen(versal_cips_0_M_AXI_FPD_ARLEN),
        .S00_AXI_arlock(versal_cips_0_M_AXI_FPD_ARLOCK),
        .S00_AXI_arprot(versal_cips_0_M_AXI_FPD_ARPROT),
        .S00_AXI_arqos(versal_cips_0_M_AXI_FPD_ARQOS),
        .S00_AXI_arready(versal_cips_0_M_AXI_FPD_ARREADY),
        .S00_AXI_arsize(versal_cips_0_M_AXI_FPD_ARSIZE),
        .S00_AXI_aruser(versal_cips_0_M_AXI_FPD_ARUSER),
        .S00_AXI_arvalid(versal_cips_0_M_AXI_FPD_ARVALID),
        .S00_AXI_awaddr(versal_cips_0_M_AXI_FPD_AWADDR),
        .S00_AXI_awburst(versal_cips_0_M_AXI_FPD_AWBURST),
        .S00_AXI_awcache(versal_cips_0_M_AXI_FPD_AWCACHE),
        .S00_AXI_awid(versal_cips_0_M_AXI_FPD_AWID),
        .S00_AXI_awlen(versal_cips_0_M_AXI_FPD_AWLEN),
        .S00_AXI_awlock(versal_cips_0_M_AXI_FPD_AWLOCK),
        .S00_AXI_awprot(versal_cips_0_M_AXI_FPD_AWPROT),
        .S00_AXI_awqos(versal_cips_0_M_AXI_FPD_AWQOS),
        .S00_AXI_awready(versal_cips_0_M_AXI_FPD_AWREADY),
        .S00_AXI_awsize(versal_cips_0_M_AXI_FPD_AWSIZE),
        .S00_AXI_awuser(versal_cips_0_M_AXI_FPD_AWUSER),
        .S00_AXI_awvalid(versal_cips_0_M_AXI_FPD_AWVALID),
        .S00_AXI_bid(versal_cips_0_M_AXI_FPD_BID),
        .S00_AXI_bready(versal_cips_0_M_AXI_FPD_BREADY),
        .S00_AXI_bresp(versal_cips_0_M_AXI_FPD_BRESP),
        .S00_AXI_bvalid(versal_cips_0_M_AXI_FPD_BVALID),
        .S00_AXI_rdata(versal_cips_0_M_AXI_FPD_RDATA),
        .S00_AXI_rid(versal_cips_0_M_AXI_FPD_RID),
        .S00_AXI_rlast(versal_cips_0_M_AXI_FPD_RLAST),
        .S00_AXI_rready(versal_cips_0_M_AXI_FPD_RREADY),
        .S00_AXI_rresp(versal_cips_0_M_AXI_FPD_RRESP),
        .S00_AXI_rvalid(versal_cips_0_M_AXI_FPD_RVALID),
        .S00_AXI_wdata(versal_cips_0_M_AXI_FPD_WDATA),
        .S00_AXI_wlast(versal_cips_0_M_AXI_FPD_WLAST),
        .S00_AXI_wready(versal_cips_0_M_AXI_FPD_WREADY),
        .S00_AXI_wstrb(versal_cips_0_M_AXI_FPD_WSTRB),
        .S00_AXI_wvalid(versal_cips_0_M_AXI_FPD_WVALID),
        .aclk(clk_wizard_0_clk_out1),
        .aresetn(proc_sys_reset_0_peripheral_aresetn));
  mvdrr_smartconnect_1_0 smartconnect_mem_0
       (.M00_AXI_araddr(smartconnect_mem_0_M00_AXI_ARADDR),
        .M00_AXI_arburst(smartconnect_mem_0_M00_AXI_ARBURST),
        .M00_AXI_arcache(smartconnect_mem_0_M00_AXI_ARCACHE),
        .M00_AXI_arlen(smartconnect_mem_0_M00_AXI_ARLEN),
        .M00_AXI_arlock(smartconnect_mem_0_M00_AXI_ARLOCK),
        .M00_AXI_arprot(smartconnect_mem_0_M00_AXI_ARPROT),
        .M00_AXI_arqos(smartconnect_mem_0_M00_AXI_ARQOS),
        .M00_AXI_arready(smartconnect_mem_0_M00_AXI_ARREADY),
        .M00_AXI_arsize(smartconnect_mem_0_M00_AXI_ARSIZE),
        .M00_AXI_arvalid(smartconnect_mem_0_M00_AXI_ARVALID),
        .M00_AXI_awaddr(smartconnect_mem_0_M00_AXI_AWADDR),
        .M00_AXI_awburst(smartconnect_mem_0_M00_AXI_AWBURST),
        .M00_AXI_awcache(smartconnect_mem_0_M00_AXI_AWCACHE),
        .M00_AXI_awlen(smartconnect_mem_0_M00_AXI_AWLEN),
        .M00_AXI_awlock(smartconnect_mem_0_M00_AXI_AWLOCK),
        .M00_AXI_awprot(smartconnect_mem_0_M00_AXI_AWPROT),
        .M00_AXI_awqos(smartconnect_mem_0_M00_AXI_AWQOS),
        .M00_AXI_awready(smartconnect_mem_0_M00_AXI_AWREADY),
        .M00_AXI_awsize(smartconnect_mem_0_M00_AXI_AWSIZE),
        .M00_AXI_awvalid(smartconnect_mem_0_M00_AXI_AWVALID),
        .M00_AXI_bready(smartconnect_mem_0_M00_AXI_BREADY),
        .M00_AXI_bresp(smartconnect_mem_0_M00_AXI_BRESP),
        .M00_AXI_bvalid(smartconnect_mem_0_M00_AXI_BVALID),
        .M00_AXI_rdata(smartconnect_mem_0_M00_AXI_RDATA),
        .M00_AXI_rlast(smartconnect_mem_0_M00_AXI_RLAST),
        .M00_AXI_rready(smartconnect_mem_0_M00_AXI_RREADY),
        .M00_AXI_rresp(smartconnect_mem_0_M00_AXI_RRESP),
        .M00_AXI_rvalid(smartconnect_mem_0_M00_AXI_RVALID),
        .M00_AXI_wdata(smartconnect_mem_0_M00_AXI_WDATA),
        .M00_AXI_wlast(smartconnect_mem_0_M00_AXI_WLAST),
        .M00_AXI_wready(smartconnect_mem_0_M00_AXI_WREADY),
        .M00_AXI_wstrb(smartconnect_mem_0_M00_AXI_WSTRB),
        .M00_AXI_wvalid(smartconnect_mem_0_M00_AXI_WVALID),
        .S00_AXI_araddr(mvdr_weights_0_m_axi_gmem0_ARADDR),
        .S00_AXI_arburst(mvdr_weights_0_m_axi_gmem0_ARBURST),
        .S00_AXI_arcache(mvdr_weights_0_m_axi_gmem0_ARCACHE),
        .S00_AXI_arid(mvdr_weights_0_m_axi_gmem0_ARID),
        .S00_AXI_arlen(mvdr_weights_0_m_axi_gmem0_ARLEN),
        .S00_AXI_arlock(mvdr_weights_0_m_axi_gmem0_ARLOCK[0]),
        .S00_AXI_arprot(mvdr_weights_0_m_axi_gmem0_ARPROT),
        .S00_AXI_arqos(mvdr_weights_0_m_axi_gmem0_ARQOS),
        .S00_AXI_arready(mvdr_weights_0_m_axi_gmem0_ARREADY),
        .S00_AXI_arsize(mvdr_weights_0_m_axi_gmem0_ARSIZE),
        .S00_AXI_arvalid(mvdr_weights_0_m_axi_gmem0_ARVALID),
        .S00_AXI_rdata(mvdr_weights_0_m_axi_gmem0_RDATA),
        .S00_AXI_rid(mvdr_weights_0_m_axi_gmem0_RID),
        .S00_AXI_rlast(mvdr_weights_0_m_axi_gmem0_RLAST),
        .S00_AXI_rready(mvdr_weights_0_m_axi_gmem0_RREADY),
        .S00_AXI_rresp(mvdr_weights_0_m_axi_gmem0_RRESP),
        .S00_AXI_rvalid(mvdr_weights_0_m_axi_gmem0_RVALID),
        .S01_AXI_araddr(mvdr_weights_0_m_axi_gmem1_ARADDR),
        .S01_AXI_arburst(mvdr_weights_0_m_axi_gmem1_ARBURST),
        .S01_AXI_arcache(mvdr_weights_0_m_axi_gmem1_ARCACHE),
        .S01_AXI_arid(mvdr_weights_0_m_axi_gmem1_ARID),
        .S01_AXI_arlen(mvdr_weights_0_m_axi_gmem1_ARLEN),
        .S01_AXI_arlock(mvdr_weights_0_m_axi_gmem1_ARLOCK[0]),
        .S01_AXI_arprot(mvdr_weights_0_m_axi_gmem1_ARPROT),
        .S01_AXI_arqos(mvdr_weights_0_m_axi_gmem1_ARQOS),
        .S01_AXI_arready(mvdr_weights_0_m_axi_gmem1_ARREADY),
        .S01_AXI_arsize(mvdr_weights_0_m_axi_gmem1_ARSIZE),
        .S01_AXI_arvalid(mvdr_weights_0_m_axi_gmem1_ARVALID),
        .S01_AXI_rdata(mvdr_weights_0_m_axi_gmem1_RDATA),
        .S01_AXI_rid(mvdr_weights_0_m_axi_gmem1_RID),
        .S01_AXI_rlast(mvdr_weights_0_m_axi_gmem1_RLAST),
        .S01_AXI_rready(mvdr_weights_0_m_axi_gmem1_RREADY),
        .S01_AXI_rresp(mvdr_weights_0_m_axi_gmem1_RRESP),
        .S01_AXI_rvalid(mvdr_weights_0_m_axi_gmem1_RVALID),
        .S02_AXI_awaddr(mvdr_weights_0_m_axi_gmem2_AWADDR),
        .S02_AXI_awburst(mvdr_weights_0_m_axi_gmem2_AWBURST),
        .S02_AXI_awcache(mvdr_weights_0_m_axi_gmem2_AWCACHE),
        .S02_AXI_awid(mvdr_weights_0_m_axi_gmem2_AWID),
        .S02_AXI_awlen(mvdr_weights_0_m_axi_gmem2_AWLEN),
        .S02_AXI_awlock(mvdr_weights_0_m_axi_gmem2_AWLOCK[0]),
        .S02_AXI_awprot(mvdr_weights_0_m_axi_gmem2_AWPROT),
        .S02_AXI_awqos(mvdr_weights_0_m_axi_gmem2_AWQOS),
        .S02_AXI_awready(mvdr_weights_0_m_axi_gmem2_AWREADY),
        .S02_AXI_awsize(mvdr_weights_0_m_axi_gmem2_AWSIZE),
        .S02_AXI_awvalid(mvdr_weights_0_m_axi_gmem2_AWVALID),
        .S02_AXI_bid(mvdr_weights_0_m_axi_gmem2_BID),
        .S02_AXI_bready(mvdr_weights_0_m_axi_gmem2_BREADY),
        .S02_AXI_bresp(mvdr_weights_0_m_axi_gmem2_BRESP),
        .S02_AXI_bvalid(mvdr_weights_0_m_axi_gmem2_BVALID),
        .S02_AXI_wdata(mvdr_weights_0_m_axi_gmem2_WDATA),
        .S02_AXI_wlast(mvdr_weights_0_m_axi_gmem2_WLAST),
        .S02_AXI_wready(mvdr_weights_0_m_axi_gmem2_WREADY),
        .S02_AXI_wstrb(mvdr_weights_0_m_axi_gmem2_WSTRB),
        .S02_AXI_wvalid(mvdr_weights_0_m_axi_gmem2_WVALID),
        .aclk(clk_wizard_0_clk_out1),
        .aresetn(proc_sys_reset_0_peripheral_aresetn));
  mvdrr_versal_cips_0_0 versal_cips_0
       (.FPD_CCI_NOC_0_araddr(versal_cips_0_FPD_CCI_NOC_0_ARADDR),
        .FPD_CCI_NOC_0_arburst(versal_cips_0_FPD_CCI_NOC_0_ARBURST),
        .FPD_CCI_NOC_0_arcache(versal_cips_0_FPD_CCI_NOC_0_ARCACHE),
        .FPD_CCI_NOC_0_arid(versal_cips_0_FPD_CCI_NOC_0_ARID),
        .FPD_CCI_NOC_0_arlen(versal_cips_0_FPD_CCI_NOC_0_ARLEN),
        .FPD_CCI_NOC_0_arlock(versal_cips_0_FPD_CCI_NOC_0_ARLOCK),
        .FPD_CCI_NOC_0_arprot(versal_cips_0_FPD_CCI_NOC_0_ARPROT),
        .FPD_CCI_NOC_0_arqos(versal_cips_0_FPD_CCI_NOC_0_ARQOS),
        .FPD_CCI_NOC_0_arready(versal_cips_0_FPD_CCI_NOC_0_ARREADY),
        .FPD_CCI_NOC_0_arsize(versal_cips_0_FPD_CCI_NOC_0_ARSIZE),
        .FPD_CCI_NOC_0_aruser(versal_cips_0_FPD_CCI_NOC_0_ARUSER),
        .FPD_CCI_NOC_0_arvalid(versal_cips_0_FPD_CCI_NOC_0_ARVALID),
        .FPD_CCI_NOC_0_awaddr(versal_cips_0_FPD_CCI_NOC_0_AWADDR),
        .FPD_CCI_NOC_0_awburst(versal_cips_0_FPD_CCI_NOC_0_AWBURST),
        .FPD_CCI_NOC_0_awcache(versal_cips_0_FPD_CCI_NOC_0_AWCACHE),
        .FPD_CCI_NOC_0_awid(versal_cips_0_FPD_CCI_NOC_0_AWID),
        .FPD_CCI_NOC_0_awlen(versal_cips_0_FPD_CCI_NOC_0_AWLEN),
        .FPD_CCI_NOC_0_awlock(versal_cips_0_FPD_CCI_NOC_0_AWLOCK),
        .FPD_CCI_NOC_0_awprot(versal_cips_0_FPD_CCI_NOC_0_AWPROT),
        .FPD_CCI_NOC_0_awqos(versal_cips_0_FPD_CCI_NOC_0_AWQOS),
        .FPD_CCI_NOC_0_awready(versal_cips_0_FPD_CCI_NOC_0_AWREADY),
        .FPD_CCI_NOC_0_awsize(versal_cips_0_FPD_CCI_NOC_0_AWSIZE),
        .FPD_CCI_NOC_0_awuser(versal_cips_0_FPD_CCI_NOC_0_AWUSER),
        .FPD_CCI_NOC_0_awvalid(versal_cips_0_FPD_CCI_NOC_0_AWVALID),
        .FPD_CCI_NOC_0_bid(versal_cips_0_FPD_CCI_NOC_0_BID),
        .FPD_CCI_NOC_0_bready(versal_cips_0_FPD_CCI_NOC_0_BREADY),
        .FPD_CCI_NOC_0_bresp(versal_cips_0_FPD_CCI_NOC_0_BRESP),
        .FPD_CCI_NOC_0_bvalid(versal_cips_0_FPD_CCI_NOC_0_BVALID),
        .FPD_CCI_NOC_0_rdata(versal_cips_0_FPD_CCI_NOC_0_RDATA),
        .FPD_CCI_NOC_0_rid(versal_cips_0_FPD_CCI_NOC_0_RID),
        .FPD_CCI_NOC_0_rlast(versal_cips_0_FPD_CCI_NOC_0_RLAST),
        .FPD_CCI_NOC_0_rready(versal_cips_0_FPD_CCI_NOC_0_RREADY),
        .FPD_CCI_NOC_0_rresp(versal_cips_0_FPD_CCI_NOC_0_RRESP),
        .FPD_CCI_NOC_0_rvalid(versal_cips_0_FPD_CCI_NOC_0_RVALID),
        .FPD_CCI_NOC_0_wdata(versal_cips_0_FPD_CCI_NOC_0_WDATA),
        .FPD_CCI_NOC_0_wlast(versal_cips_0_FPD_CCI_NOC_0_WLAST),
        .FPD_CCI_NOC_0_wready(versal_cips_0_FPD_CCI_NOC_0_WREADY),
        .FPD_CCI_NOC_0_wstrb(versal_cips_0_FPD_CCI_NOC_0_WSTRB),
        .FPD_CCI_NOC_0_wuser(versal_cips_0_FPD_CCI_NOC_0_WUSER),
        .FPD_CCI_NOC_0_wvalid(versal_cips_0_FPD_CCI_NOC_0_WVALID),
        .FPD_CCI_NOC_1_araddr(versal_cips_0_FPD_CCI_NOC_1_ARADDR),
        .FPD_CCI_NOC_1_arburst(versal_cips_0_FPD_CCI_NOC_1_ARBURST),
        .FPD_CCI_NOC_1_arcache(versal_cips_0_FPD_CCI_NOC_1_ARCACHE),
        .FPD_CCI_NOC_1_arid(versal_cips_0_FPD_CCI_NOC_1_ARID),
        .FPD_CCI_NOC_1_arlen(versal_cips_0_FPD_CCI_NOC_1_ARLEN),
        .FPD_CCI_NOC_1_arlock(versal_cips_0_FPD_CCI_NOC_1_ARLOCK),
        .FPD_CCI_NOC_1_arprot(versal_cips_0_FPD_CCI_NOC_1_ARPROT),
        .FPD_CCI_NOC_1_arqos(versal_cips_0_FPD_CCI_NOC_1_ARQOS),
        .FPD_CCI_NOC_1_arready(versal_cips_0_FPD_CCI_NOC_1_ARREADY),
        .FPD_CCI_NOC_1_arsize(versal_cips_0_FPD_CCI_NOC_1_ARSIZE),
        .FPD_CCI_NOC_1_aruser(versal_cips_0_FPD_CCI_NOC_1_ARUSER),
        .FPD_CCI_NOC_1_arvalid(versal_cips_0_FPD_CCI_NOC_1_ARVALID),
        .FPD_CCI_NOC_1_awaddr(versal_cips_0_FPD_CCI_NOC_1_AWADDR),
        .FPD_CCI_NOC_1_awburst(versal_cips_0_FPD_CCI_NOC_1_AWBURST),
        .FPD_CCI_NOC_1_awcache(versal_cips_0_FPD_CCI_NOC_1_AWCACHE),
        .FPD_CCI_NOC_1_awid(versal_cips_0_FPD_CCI_NOC_1_AWID),
        .FPD_CCI_NOC_1_awlen(versal_cips_0_FPD_CCI_NOC_1_AWLEN),
        .FPD_CCI_NOC_1_awlock(versal_cips_0_FPD_CCI_NOC_1_AWLOCK),
        .FPD_CCI_NOC_1_awprot(versal_cips_0_FPD_CCI_NOC_1_AWPROT),
        .FPD_CCI_NOC_1_awqos(versal_cips_0_FPD_CCI_NOC_1_AWQOS),
        .FPD_CCI_NOC_1_awready(versal_cips_0_FPD_CCI_NOC_1_AWREADY),
        .FPD_CCI_NOC_1_awsize(versal_cips_0_FPD_CCI_NOC_1_AWSIZE),
        .FPD_CCI_NOC_1_awuser(versal_cips_0_FPD_CCI_NOC_1_AWUSER),
        .FPD_CCI_NOC_1_awvalid(versal_cips_0_FPD_CCI_NOC_1_AWVALID),
        .FPD_CCI_NOC_1_bid(versal_cips_0_FPD_CCI_NOC_1_BID),
        .FPD_CCI_NOC_1_bready(versal_cips_0_FPD_CCI_NOC_1_BREADY),
        .FPD_CCI_NOC_1_bresp(versal_cips_0_FPD_CCI_NOC_1_BRESP),
        .FPD_CCI_NOC_1_bvalid(versal_cips_0_FPD_CCI_NOC_1_BVALID),
        .FPD_CCI_NOC_1_rdata(versal_cips_0_FPD_CCI_NOC_1_RDATA),
        .FPD_CCI_NOC_1_rid(versal_cips_0_FPD_CCI_NOC_1_RID),
        .FPD_CCI_NOC_1_rlast(versal_cips_0_FPD_CCI_NOC_1_RLAST),
        .FPD_CCI_NOC_1_rready(versal_cips_0_FPD_CCI_NOC_1_RREADY),
        .FPD_CCI_NOC_1_rresp(versal_cips_0_FPD_CCI_NOC_1_RRESP),
        .FPD_CCI_NOC_1_rvalid(versal_cips_0_FPD_CCI_NOC_1_RVALID),
        .FPD_CCI_NOC_1_wdata(versal_cips_0_FPD_CCI_NOC_1_WDATA),
        .FPD_CCI_NOC_1_wlast(versal_cips_0_FPD_CCI_NOC_1_WLAST),
        .FPD_CCI_NOC_1_wready(versal_cips_0_FPD_CCI_NOC_1_WREADY),
        .FPD_CCI_NOC_1_wstrb(versal_cips_0_FPD_CCI_NOC_1_WSTRB),
        .FPD_CCI_NOC_1_wuser(versal_cips_0_FPD_CCI_NOC_1_WUSER),
        .FPD_CCI_NOC_1_wvalid(versal_cips_0_FPD_CCI_NOC_1_WVALID),
        .FPD_CCI_NOC_2_araddr(versal_cips_0_FPD_CCI_NOC_2_ARADDR),
        .FPD_CCI_NOC_2_arburst(versal_cips_0_FPD_CCI_NOC_2_ARBURST),
        .FPD_CCI_NOC_2_arcache(versal_cips_0_FPD_CCI_NOC_2_ARCACHE),
        .FPD_CCI_NOC_2_arid(versal_cips_0_FPD_CCI_NOC_2_ARID),
        .FPD_CCI_NOC_2_arlen(versal_cips_0_FPD_CCI_NOC_2_ARLEN),
        .FPD_CCI_NOC_2_arlock(versal_cips_0_FPD_CCI_NOC_2_ARLOCK),
        .FPD_CCI_NOC_2_arprot(versal_cips_0_FPD_CCI_NOC_2_ARPROT),
        .FPD_CCI_NOC_2_arqos(versal_cips_0_FPD_CCI_NOC_2_ARQOS),
        .FPD_CCI_NOC_2_arready(versal_cips_0_FPD_CCI_NOC_2_ARREADY),
        .FPD_CCI_NOC_2_arsize(versal_cips_0_FPD_CCI_NOC_2_ARSIZE),
        .FPD_CCI_NOC_2_aruser(versal_cips_0_FPD_CCI_NOC_2_ARUSER),
        .FPD_CCI_NOC_2_arvalid(versal_cips_0_FPD_CCI_NOC_2_ARVALID),
        .FPD_CCI_NOC_2_awaddr(versal_cips_0_FPD_CCI_NOC_2_AWADDR),
        .FPD_CCI_NOC_2_awburst(versal_cips_0_FPD_CCI_NOC_2_AWBURST),
        .FPD_CCI_NOC_2_awcache(versal_cips_0_FPD_CCI_NOC_2_AWCACHE),
        .FPD_CCI_NOC_2_awid(versal_cips_0_FPD_CCI_NOC_2_AWID),
        .FPD_CCI_NOC_2_awlen(versal_cips_0_FPD_CCI_NOC_2_AWLEN),
        .FPD_CCI_NOC_2_awlock(versal_cips_0_FPD_CCI_NOC_2_AWLOCK),
        .FPD_CCI_NOC_2_awprot(versal_cips_0_FPD_CCI_NOC_2_AWPROT),
        .FPD_CCI_NOC_2_awqos(versal_cips_0_FPD_CCI_NOC_2_AWQOS),
        .FPD_CCI_NOC_2_awready(versal_cips_0_FPD_CCI_NOC_2_AWREADY),
        .FPD_CCI_NOC_2_awsize(versal_cips_0_FPD_CCI_NOC_2_AWSIZE),
        .FPD_CCI_NOC_2_awuser(versal_cips_0_FPD_CCI_NOC_2_AWUSER),
        .FPD_CCI_NOC_2_awvalid(versal_cips_0_FPD_CCI_NOC_2_AWVALID),
        .FPD_CCI_NOC_2_bid(versal_cips_0_FPD_CCI_NOC_2_BID),
        .FPD_CCI_NOC_2_bready(versal_cips_0_FPD_CCI_NOC_2_BREADY),
        .FPD_CCI_NOC_2_bresp(versal_cips_0_FPD_CCI_NOC_2_BRESP),
        .FPD_CCI_NOC_2_bvalid(versal_cips_0_FPD_CCI_NOC_2_BVALID),
        .FPD_CCI_NOC_2_rdata(versal_cips_0_FPD_CCI_NOC_2_RDATA),
        .FPD_CCI_NOC_2_rid(versal_cips_0_FPD_CCI_NOC_2_RID),
        .FPD_CCI_NOC_2_rlast(versal_cips_0_FPD_CCI_NOC_2_RLAST),
        .FPD_CCI_NOC_2_rready(versal_cips_0_FPD_CCI_NOC_2_RREADY),
        .FPD_CCI_NOC_2_rresp(versal_cips_0_FPD_CCI_NOC_2_RRESP),
        .FPD_CCI_NOC_2_rvalid(versal_cips_0_FPD_CCI_NOC_2_RVALID),
        .FPD_CCI_NOC_2_wdata(versal_cips_0_FPD_CCI_NOC_2_WDATA),
        .FPD_CCI_NOC_2_wlast(versal_cips_0_FPD_CCI_NOC_2_WLAST),
        .FPD_CCI_NOC_2_wready(versal_cips_0_FPD_CCI_NOC_2_WREADY),
        .FPD_CCI_NOC_2_wstrb(versal_cips_0_FPD_CCI_NOC_2_WSTRB),
        .FPD_CCI_NOC_2_wuser(versal_cips_0_FPD_CCI_NOC_2_WUSER),
        .FPD_CCI_NOC_2_wvalid(versal_cips_0_FPD_CCI_NOC_2_WVALID),
        .FPD_CCI_NOC_3_araddr(versal_cips_0_FPD_CCI_NOC_3_ARADDR),
        .FPD_CCI_NOC_3_arburst(versal_cips_0_FPD_CCI_NOC_3_ARBURST),
        .FPD_CCI_NOC_3_arcache(versal_cips_0_FPD_CCI_NOC_3_ARCACHE),
        .FPD_CCI_NOC_3_arid(versal_cips_0_FPD_CCI_NOC_3_ARID),
        .FPD_CCI_NOC_3_arlen(versal_cips_0_FPD_CCI_NOC_3_ARLEN),
        .FPD_CCI_NOC_3_arlock(versal_cips_0_FPD_CCI_NOC_3_ARLOCK),
        .FPD_CCI_NOC_3_arprot(versal_cips_0_FPD_CCI_NOC_3_ARPROT),
        .FPD_CCI_NOC_3_arqos(versal_cips_0_FPD_CCI_NOC_3_ARQOS),
        .FPD_CCI_NOC_3_arready(versal_cips_0_FPD_CCI_NOC_3_ARREADY),
        .FPD_CCI_NOC_3_arsize(versal_cips_0_FPD_CCI_NOC_3_ARSIZE),
        .FPD_CCI_NOC_3_aruser(versal_cips_0_FPD_CCI_NOC_3_ARUSER),
        .FPD_CCI_NOC_3_arvalid(versal_cips_0_FPD_CCI_NOC_3_ARVALID),
        .FPD_CCI_NOC_3_awaddr(versal_cips_0_FPD_CCI_NOC_3_AWADDR),
        .FPD_CCI_NOC_3_awburst(versal_cips_0_FPD_CCI_NOC_3_AWBURST),
        .FPD_CCI_NOC_3_awcache(versal_cips_0_FPD_CCI_NOC_3_AWCACHE),
        .FPD_CCI_NOC_3_awid(versal_cips_0_FPD_CCI_NOC_3_AWID),
        .FPD_CCI_NOC_3_awlen(versal_cips_0_FPD_CCI_NOC_3_AWLEN),
        .FPD_CCI_NOC_3_awlock(versal_cips_0_FPD_CCI_NOC_3_AWLOCK),
        .FPD_CCI_NOC_3_awprot(versal_cips_0_FPD_CCI_NOC_3_AWPROT),
        .FPD_CCI_NOC_3_awqos(versal_cips_0_FPD_CCI_NOC_3_AWQOS),
        .FPD_CCI_NOC_3_awready(versal_cips_0_FPD_CCI_NOC_3_AWREADY),
        .FPD_CCI_NOC_3_awsize(versal_cips_0_FPD_CCI_NOC_3_AWSIZE),
        .FPD_CCI_NOC_3_awuser(versal_cips_0_FPD_CCI_NOC_3_AWUSER),
        .FPD_CCI_NOC_3_awvalid(versal_cips_0_FPD_CCI_NOC_3_AWVALID),
        .FPD_CCI_NOC_3_bid(versal_cips_0_FPD_CCI_NOC_3_BID),
        .FPD_CCI_NOC_3_bready(versal_cips_0_FPD_CCI_NOC_3_BREADY),
        .FPD_CCI_NOC_3_bresp(versal_cips_0_FPD_CCI_NOC_3_BRESP),
        .FPD_CCI_NOC_3_bvalid(versal_cips_0_FPD_CCI_NOC_3_BVALID),
        .FPD_CCI_NOC_3_rdata(versal_cips_0_FPD_CCI_NOC_3_RDATA),
        .FPD_CCI_NOC_3_rid(versal_cips_0_FPD_CCI_NOC_3_RID),
        .FPD_CCI_NOC_3_rlast(versal_cips_0_FPD_CCI_NOC_3_RLAST),
        .FPD_CCI_NOC_3_rready(versal_cips_0_FPD_CCI_NOC_3_RREADY),
        .FPD_CCI_NOC_3_rresp(versal_cips_0_FPD_CCI_NOC_3_RRESP),
        .FPD_CCI_NOC_3_rvalid(versal_cips_0_FPD_CCI_NOC_3_RVALID),
        .FPD_CCI_NOC_3_wdata(versal_cips_0_FPD_CCI_NOC_3_WDATA),
        .FPD_CCI_NOC_3_wlast(versal_cips_0_FPD_CCI_NOC_3_WLAST),
        .FPD_CCI_NOC_3_wready(versal_cips_0_FPD_CCI_NOC_3_WREADY),
        .FPD_CCI_NOC_3_wstrb(versal_cips_0_FPD_CCI_NOC_3_WSTRB),
        .FPD_CCI_NOC_3_wuser(versal_cips_0_FPD_CCI_NOC_3_WUSER),
        .FPD_CCI_NOC_3_wvalid(versal_cips_0_FPD_CCI_NOC_3_WVALID),
        .LPD_AXI_NOC_0_araddr(versal_cips_0_LPD_AXI_NOC_0_ARADDR),
        .LPD_AXI_NOC_0_arburst(versal_cips_0_LPD_AXI_NOC_0_ARBURST),
        .LPD_AXI_NOC_0_arcache(versal_cips_0_LPD_AXI_NOC_0_ARCACHE),
        .LPD_AXI_NOC_0_arid(versal_cips_0_LPD_AXI_NOC_0_ARID),
        .LPD_AXI_NOC_0_arlen(versal_cips_0_LPD_AXI_NOC_0_ARLEN),
        .LPD_AXI_NOC_0_arlock(versal_cips_0_LPD_AXI_NOC_0_ARLOCK),
        .LPD_AXI_NOC_0_arprot(versal_cips_0_LPD_AXI_NOC_0_ARPROT),
        .LPD_AXI_NOC_0_arqos(versal_cips_0_LPD_AXI_NOC_0_ARQOS),
        .LPD_AXI_NOC_0_arready(versal_cips_0_LPD_AXI_NOC_0_ARREADY),
        .LPD_AXI_NOC_0_arsize(versal_cips_0_LPD_AXI_NOC_0_ARSIZE),
        .LPD_AXI_NOC_0_aruser(versal_cips_0_LPD_AXI_NOC_0_ARUSER),
        .LPD_AXI_NOC_0_arvalid(versal_cips_0_LPD_AXI_NOC_0_ARVALID),
        .LPD_AXI_NOC_0_awaddr(versal_cips_0_LPD_AXI_NOC_0_AWADDR),
        .LPD_AXI_NOC_0_awburst(versal_cips_0_LPD_AXI_NOC_0_AWBURST),
        .LPD_AXI_NOC_0_awcache(versal_cips_0_LPD_AXI_NOC_0_AWCACHE),
        .LPD_AXI_NOC_0_awid(versal_cips_0_LPD_AXI_NOC_0_AWID),
        .LPD_AXI_NOC_0_awlen(versal_cips_0_LPD_AXI_NOC_0_AWLEN),
        .LPD_AXI_NOC_0_awlock(versal_cips_0_LPD_AXI_NOC_0_AWLOCK),
        .LPD_AXI_NOC_0_awprot(versal_cips_0_LPD_AXI_NOC_0_AWPROT),
        .LPD_AXI_NOC_0_awqos(versal_cips_0_LPD_AXI_NOC_0_AWQOS),
        .LPD_AXI_NOC_0_awready(versal_cips_0_LPD_AXI_NOC_0_AWREADY),
        .LPD_AXI_NOC_0_awsize(versal_cips_0_LPD_AXI_NOC_0_AWSIZE),
        .LPD_AXI_NOC_0_awuser(versal_cips_0_LPD_AXI_NOC_0_AWUSER),
        .LPD_AXI_NOC_0_awvalid(versal_cips_0_LPD_AXI_NOC_0_AWVALID),
        .LPD_AXI_NOC_0_bid(versal_cips_0_LPD_AXI_NOC_0_BID),
        .LPD_AXI_NOC_0_bready(versal_cips_0_LPD_AXI_NOC_0_BREADY),
        .LPD_AXI_NOC_0_bresp(versal_cips_0_LPD_AXI_NOC_0_BRESP),
        .LPD_AXI_NOC_0_bvalid(versal_cips_0_LPD_AXI_NOC_0_BVALID),
        .LPD_AXI_NOC_0_rdata(versal_cips_0_LPD_AXI_NOC_0_RDATA),
        .LPD_AXI_NOC_0_rid(versal_cips_0_LPD_AXI_NOC_0_RID),
        .LPD_AXI_NOC_0_rlast(versal_cips_0_LPD_AXI_NOC_0_RLAST),
        .LPD_AXI_NOC_0_rready(versal_cips_0_LPD_AXI_NOC_0_RREADY),
        .LPD_AXI_NOC_0_rresp(versal_cips_0_LPD_AXI_NOC_0_RRESP),
        .LPD_AXI_NOC_0_rvalid(versal_cips_0_LPD_AXI_NOC_0_RVALID),
        .LPD_AXI_NOC_0_wdata(versal_cips_0_LPD_AXI_NOC_0_WDATA),
        .LPD_AXI_NOC_0_wlast(versal_cips_0_LPD_AXI_NOC_0_WLAST),
        .LPD_AXI_NOC_0_wready(versal_cips_0_LPD_AXI_NOC_0_WREADY),
        .LPD_AXI_NOC_0_wstrb(versal_cips_0_LPD_AXI_NOC_0_WSTRB),
        .LPD_AXI_NOC_0_wvalid(versal_cips_0_LPD_AXI_NOC_0_WVALID),
        .M_AXI_FPD_araddr(versal_cips_0_M_AXI_FPD_ARADDR),
        .M_AXI_FPD_arburst(versal_cips_0_M_AXI_FPD_ARBURST),
        .M_AXI_FPD_arcache(versal_cips_0_M_AXI_FPD_ARCACHE),
        .M_AXI_FPD_arid(versal_cips_0_M_AXI_FPD_ARID),
        .M_AXI_FPD_arlen(versal_cips_0_M_AXI_FPD_ARLEN),
        .M_AXI_FPD_arlock(versal_cips_0_M_AXI_FPD_ARLOCK),
        .M_AXI_FPD_arprot(versal_cips_0_M_AXI_FPD_ARPROT),
        .M_AXI_FPD_arqos(versal_cips_0_M_AXI_FPD_ARQOS),
        .M_AXI_FPD_arready(versal_cips_0_M_AXI_FPD_ARREADY),
        .M_AXI_FPD_arsize(versal_cips_0_M_AXI_FPD_ARSIZE),
        .M_AXI_FPD_aruser(versal_cips_0_M_AXI_FPD_ARUSER),
        .M_AXI_FPD_arvalid(versal_cips_0_M_AXI_FPD_ARVALID),
        .M_AXI_FPD_awaddr(versal_cips_0_M_AXI_FPD_AWADDR),
        .M_AXI_FPD_awburst(versal_cips_0_M_AXI_FPD_AWBURST),
        .M_AXI_FPD_awcache(versal_cips_0_M_AXI_FPD_AWCACHE),
        .M_AXI_FPD_awid(versal_cips_0_M_AXI_FPD_AWID),
        .M_AXI_FPD_awlen(versal_cips_0_M_AXI_FPD_AWLEN),
        .M_AXI_FPD_awlock(versal_cips_0_M_AXI_FPD_AWLOCK),
        .M_AXI_FPD_awprot(versal_cips_0_M_AXI_FPD_AWPROT),
        .M_AXI_FPD_awqos(versal_cips_0_M_AXI_FPD_AWQOS),
        .M_AXI_FPD_awready(versal_cips_0_M_AXI_FPD_AWREADY),
        .M_AXI_FPD_awsize(versal_cips_0_M_AXI_FPD_AWSIZE),
        .M_AXI_FPD_awuser(versal_cips_0_M_AXI_FPD_AWUSER),
        .M_AXI_FPD_awvalid(versal_cips_0_M_AXI_FPD_AWVALID),
        .M_AXI_FPD_bid(versal_cips_0_M_AXI_FPD_BID),
        .M_AXI_FPD_bready(versal_cips_0_M_AXI_FPD_BREADY),
        .M_AXI_FPD_bresp(versal_cips_0_M_AXI_FPD_BRESP),
        .M_AXI_FPD_bvalid(versal_cips_0_M_AXI_FPD_BVALID),
        .M_AXI_FPD_rdata(versal_cips_0_M_AXI_FPD_RDATA),
        .M_AXI_FPD_rid(versal_cips_0_M_AXI_FPD_RID),
        .M_AXI_FPD_rlast(versal_cips_0_M_AXI_FPD_RLAST),
        .M_AXI_FPD_rready(versal_cips_0_M_AXI_FPD_RREADY),
        .M_AXI_FPD_rresp(versal_cips_0_M_AXI_FPD_RRESP),
        .M_AXI_FPD_rvalid(versal_cips_0_M_AXI_FPD_RVALID),
        .M_AXI_FPD_wdata(versal_cips_0_M_AXI_FPD_WDATA),
        .M_AXI_FPD_wlast(versal_cips_0_M_AXI_FPD_WLAST),
        .M_AXI_FPD_wready(versal_cips_0_M_AXI_FPD_WREADY),
        .M_AXI_FPD_wstrb(versal_cips_0_M_AXI_FPD_WSTRB),
        .M_AXI_FPD_wvalid(versal_cips_0_M_AXI_FPD_WVALID),
        .PMC_NOC_AXI_0_araddr(versal_cips_0_PMC_NOC_AXI_0_ARADDR),
        .PMC_NOC_AXI_0_arburst(versal_cips_0_PMC_NOC_AXI_0_ARBURST),
        .PMC_NOC_AXI_0_arcache(versal_cips_0_PMC_NOC_AXI_0_ARCACHE),
        .PMC_NOC_AXI_0_arid(versal_cips_0_PMC_NOC_AXI_0_ARID),
        .PMC_NOC_AXI_0_arlen(versal_cips_0_PMC_NOC_AXI_0_ARLEN),
        .PMC_NOC_AXI_0_arlock(versal_cips_0_PMC_NOC_AXI_0_ARLOCK),
        .PMC_NOC_AXI_0_arprot(versal_cips_0_PMC_NOC_AXI_0_ARPROT),
        .PMC_NOC_AXI_0_arqos(versal_cips_0_PMC_NOC_AXI_0_ARQOS),
        .PMC_NOC_AXI_0_arready(versal_cips_0_PMC_NOC_AXI_0_ARREADY),
        .PMC_NOC_AXI_0_arregion(versal_cips_0_PMC_NOC_AXI_0_ARREGION),
        .PMC_NOC_AXI_0_arsize(versal_cips_0_PMC_NOC_AXI_0_ARSIZE),
        .PMC_NOC_AXI_0_aruser(versal_cips_0_PMC_NOC_AXI_0_ARUSER),
        .PMC_NOC_AXI_0_arvalid(versal_cips_0_PMC_NOC_AXI_0_ARVALID),
        .PMC_NOC_AXI_0_awaddr(versal_cips_0_PMC_NOC_AXI_0_AWADDR),
        .PMC_NOC_AXI_0_awburst(versal_cips_0_PMC_NOC_AXI_0_AWBURST),
        .PMC_NOC_AXI_0_awcache(versal_cips_0_PMC_NOC_AXI_0_AWCACHE),
        .PMC_NOC_AXI_0_awid(versal_cips_0_PMC_NOC_AXI_0_AWID),
        .PMC_NOC_AXI_0_awlen(versal_cips_0_PMC_NOC_AXI_0_AWLEN),
        .PMC_NOC_AXI_0_awlock(versal_cips_0_PMC_NOC_AXI_0_AWLOCK),
        .PMC_NOC_AXI_0_awprot(versal_cips_0_PMC_NOC_AXI_0_AWPROT),
        .PMC_NOC_AXI_0_awqos(versal_cips_0_PMC_NOC_AXI_0_AWQOS),
        .PMC_NOC_AXI_0_awready(versal_cips_0_PMC_NOC_AXI_0_AWREADY),
        .PMC_NOC_AXI_0_awregion(versal_cips_0_PMC_NOC_AXI_0_AWREGION),
        .PMC_NOC_AXI_0_awsize(versal_cips_0_PMC_NOC_AXI_0_AWSIZE),
        .PMC_NOC_AXI_0_awuser(versal_cips_0_PMC_NOC_AXI_0_AWUSER),
        .PMC_NOC_AXI_0_awvalid(versal_cips_0_PMC_NOC_AXI_0_AWVALID),
        .PMC_NOC_AXI_0_bid(versal_cips_0_PMC_NOC_AXI_0_BID),
        .PMC_NOC_AXI_0_bready(versal_cips_0_PMC_NOC_AXI_0_BREADY),
        .PMC_NOC_AXI_0_bresp(versal_cips_0_PMC_NOC_AXI_0_BRESP),
        .PMC_NOC_AXI_0_buser(versal_cips_0_PMC_NOC_AXI_0_BUSER),
        .PMC_NOC_AXI_0_bvalid(versal_cips_0_PMC_NOC_AXI_0_BVALID),
        .PMC_NOC_AXI_0_rdata(versal_cips_0_PMC_NOC_AXI_0_RDATA),
        .PMC_NOC_AXI_0_rid(versal_cips_0_PMC_NOC_AXI_0_RID),
        .PMC_NOC_AXI_0_rlast(versal_cips_0_PMC_NOC_AXI_0_RLAST),
        .PMC_NOC_AXI_0_rready(versal_cips_0_PMC_NOC_AXI_0_RREADY),
        .PMC_NOC_AXI_0_rresp(versal_cips_0_PMC_NOC_AXI_0_RRESP),
        .PMC_NOC_AXI_0_ruser(versal_cips_0_PMC_NOC_AXI_0_RUSER),
        .PMC_NOC_AXI_0_rvalid(versal_cips_0_PMC_NOC_AXI_0_RVALID),
        .PMC_NOC_AXI_0_wdata(versal_cips_0_PMC_NOC_AXI_0_WDATA),
        .PMC_NOC_AXI_0_wlast(versal_cips_0_PMC_NOC_AXI_0_WLAST),
        .PMC_NOC_AXI_0_wready(versal_cips_0_PMC_NOC_AXI_0_WREADY),
        .PMC_NOC_AXI_0_wstrb(versal_cips_0_PMC_NOC_AXI_0_WSTRB),
        .PMC_NOC_AXI_0_wuser(versal_cips_0_PMC_NOC_AXI_0_WUSER),
        .PMC_NOC_AXI_0_wvalid(versal_cips_0_PMC_NOC_AXI_0_WVALID),
        .fpd_cci_noc_axi0_clk(versal_cips_0_fpd_cci_noc_axi0_clk),
        .fpd_cci_noc_axi1_clk(versal_cips_0_fpd_cci_noc_axi1_clk),
        .fpd_cci_noc_axi2_clk(versal_cips_0_fpd_cci_noc_axi2_clk),
        .fpd_cci_noc_axi3_clk(versal_cips_0_fpd_cci_noc_axi3_clk),
        .lpd_axi_noc_clk(versal_cips_0_lpd_axi_noc_clk),
        .m_axi_fpd_aclk(clk_wizard_0_clk_out1),
        .pl0_ref_clk(versal_cips_0_pl0_ref_clk),
        .pl0_resetn(versal_cips_0_pl0_resetn),
        .pl_ps_irq0(rdbs_kernel_0_interrupt),
        .pl_ps_irq1(covariance_estimation_0_interrupt),
        .pl_ps_irq2(mvdr_weights_0_interrupt),
        .pl_ps_irq3(axi_dma_0_mm2s_introut),
        .pmc_axi_noc_axi0_clk(versal_cips_0_pmc_axi_noc_axi0_clk));
endmodule
