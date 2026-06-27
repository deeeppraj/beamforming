// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xcovariance_estimation.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XCovariance_estimation_CfgInitialize(XCovariance_estimation *InstancePtr, XCovariance_estimation_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XCovariance_estimation_Start(XCovariance_estimation *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_AP_CTRL) & 0x80;
    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XCovariance_estimation_IsDone(XCovariance_estimation *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XCovariance_estimation_IsIdle(XCovariance_estimation *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XCovariance_estimation_IsReady(XCovariance_estimation *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XCovariance_estimation_EnableAutoRestart(XCovariance_estimation *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_AP_CTRL, 0x80);
}

void XCovariance_estimation_DisableAutoRestart(XCovariance_estimation *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_AP_CTRL, 0);
}

void XCovariance_estimation_Set_R(XCovariance_estimation *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_R_DATA, (u32)(Data));
    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_R_DATA + 4, (u32)(Data >> 32));
}

u64 XCovariance_estimation_Get_R(XCovariance_estimation *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_R_DATA);
    Data += (u64)XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_R_DATA + 4) << 32;
    return Data;
}

void XCovariance_estimation_InterruptGlobalEnable(XCovariance_estimation *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_GIE, 1);
}

void XCovariance_estimation_InterruptGlobalDisable(XCovariance_estimation *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_GIE, 0);
}

void XCovariance_estimation_InterruptEnable(XCovariance_estimation *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_IER);
    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_IER, Register | Mask);
}

void XCovariance_estimation_InterruptDisable(XCovariance_estimation *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_IER);
    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_IER, Register & (~Mask));
}

void XCovariance_estimation_InterruptClear(XCovariance_estimation *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCovariance_estimation_WriteReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_ISR, Mask);
}

u32 XCovariance_estimation_InterruptGetEnabled(XCovariance_estimation *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_IER);
}

u32 XCovariance_estimation_InterruptGetStatus(XCovariance_estimation *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XCovariance_estimation_ReadReg(InstancePtr->Ctrl_BaseAddress, XCOVARIANCE_ESTIMATION_CTRL_ADDR_ISR);
}

