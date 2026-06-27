// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xrdbs_kernel.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XRdbs_kernel_CfgInitialize(XRdbs_kernel *InstancePtr, XRdbs_kernel_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XRdbs_kernel_Start(XRdbs_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRdbs_kernel_ReadReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_AP_CTRL) & 0x80;
    XRdbs_kernel_WriteReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XRdbs_kernel_IsDone(XRdbs_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRdbs_kernel_ReadReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XRdbs_kernel_IsIdle(XRdbs_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRdbs_kernel_ReadReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XRdbs_kernel_IsReady(XRdbs_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRdbs_kernel_ReadReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XRdbs_kernel_EnableAutoRestart(XRdbs_kernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRdbs_kernel_WriteReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XRdbs_kernel_DisableAutoRestart(XRdbs_kernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRdbs_kernel_WriteReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_AP_CTRL, 0);
}

void XRdbs_kernel_Set_num_snapshots(XRdbs_kernel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRdbs_kernel_WriteReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_NUM_SNAPSHOTS_DATA, Data);
}

u32 XRdbs_kernel_Get_num_snapshots(XRdbs_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRdbs_kernel_ReadReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_NUM_SNAPSHOTS_DATA);
    return Data;
}

void XRdbs_kernel_InterruptGlobalEnable(XRdbs_kernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRdbs_kernel_WriteReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_GIE, 1);
}

void XRdbs_kernel_InterruptGlobalDisable(XRdbs_kernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRdbs_kernel_WriteReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_GIE, 0);
}

void XRdbs_kernel_InterruptEnable(XRdbs_kernel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XRdbs_kernel_ReadReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_IER);
    XRdbs_kernel_WriteReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_IER, Register | Mask);
}

void XRdbs_kernel_InterruptDisable(XRdbs_kernel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XRdbs_kernel_ReadReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_IER);
    XRdbs_kernel_WriteReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_IER, Register & (~Mask));
}

void XRdbs_kernel_InterruptClear(XRdbs_kernel *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRdbs_kernel_WriteReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_ISR, Mask);
}

u32 XRdbs_kernel_InterruptGetEnabled(XRdbs_kernel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XRdbs_kernel_ReadReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_IER);
}

u32 XRdbs_kernel_InterruptGetStatus(XRdbs_kernel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XRdbs_kernel_ReadReg(InstancePtr->Control_BaseAddress, XRDBS_KERNEL_CONTROL_ADDR_ISR);
}

