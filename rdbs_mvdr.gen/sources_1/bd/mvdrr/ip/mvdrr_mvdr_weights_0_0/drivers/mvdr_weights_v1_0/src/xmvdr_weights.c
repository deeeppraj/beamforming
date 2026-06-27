// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xmvdr_weights.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XMvdr_weights_CfgInitialize(XMvdr_weights *InstancePtr, XMvdr_weights_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XMvdr_weights_Start(XMvdr_weights *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_AP_CTRL) & 0x80;
    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XMvdr_weights_IsDone(XMvdr_weights *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XMvdr_weights_IsIdle(XMvdr_weights *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XMvdr_weights_IsReady(XMvdr_weights *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XMvdr_weights_EnableAutoRestart(XMvdr_weights *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_AP_CTRL, 0x80);
}

void XMvdr_weights_DisableAutoRestart(XMvdr_weights *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_AP_CTRL, 0);
}

void XMvdr_weights_Set_R_in(XMvdr_weights *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_R_IN_DATA, (u32)(Data));
    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_R_IN_DATA + 4, (u32)(Data >> 32));
}

u64 XMvdr_weights_Get_R_in(XMvdr_weights *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_R_IN_DATA);
    Data += (u64)XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_R_IN_DATA + 4) << 32;
    return Data;
}

void XMvdr_weights_Set_a_in(XMvdr_weights *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_A_IN_DATA, (u32)(Data));
    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_A_IN_DATA + 4, (u32)(Data >> 32));
}

u64 XMvdr_weights_Get_a_in(XMvdr_weights *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_A_IN_DATA);
    Data += (u64)XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_A_IN_DATA + 4) << 32;
    return Data;
}

void XMvdr_weights_Set_w_out(XMvdr_weights *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_W_OUT_DATA, (u32)(Data));
    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_W_OUT_DATA + 4, (u32)(Data >> 32));
}

u64 XMvdr_weights_Get_w_out(XMvdr_weights *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_W_OUT_DATA);
    Data += (u64)XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_W_OUT_DATA + 4) << 32;
    return Data;
}

void XMvdr_weights_InterruptGlobalEnable(XMvdr_weights *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_GIE, 1);
}

void XMvdr_weights_InterruptGlobalDisable(XMvdr_weights *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_GIE, 0);
}

void XMvdr_weights_InterruptEnable(XMvdr_weights *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_IER);
    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_IER, Register | Mask);
}

void XMvdr_weights_InterruptDisable(XMvdr_weights *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_IER);
    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_IER, Register & (~Mask));
}

void XMvdr_weights_InterruptClear(XMvdr_weights *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMvdr_weights_WriteReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_ISR, Mask);
}

u32 XMvdr_weights_InterruptGetEnabled(XMvdr_weights *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_IER);
}

u32 XMvdr_weights_InterruptGetStatus(XMvdr_weights *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMvdr_weights_ReadReg(InstancePtr->Ctrl_BaseAddress, XMVDR_WEIGHTS_CTRL_ADDR_ISR);
}

