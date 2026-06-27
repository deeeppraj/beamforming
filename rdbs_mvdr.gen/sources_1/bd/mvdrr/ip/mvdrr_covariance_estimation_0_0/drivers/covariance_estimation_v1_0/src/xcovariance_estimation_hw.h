// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// ctrl
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of R
//        bit 31~0 - R[31:0] (Read/Write)
// 0x14 : Data signal of R
//        bit 31~0 - R[63:32] (Read/Write)
// 0x18 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XCOVARIANCE_ESTIMATION_CTRL_ADDR_AP_CTRL 0x00
#define XCOVARIANCE_ESTIMATION_CTRL_ADDR_GIE     0x04
#define XCOVARIANCE_ESTIMATION_CTRL_ADDR_IER     0x08
#define XCOVARIANCE_ESTIMATION_CTRL_ADDR_ISR     0x0c
#define XCOVARIANCE_ESTIMATION_CTRL_ADDR_R_DATA  0x10
#define XCOVARIANCE_ESTIMATION_CTRL_BITS_R_DATA  64

