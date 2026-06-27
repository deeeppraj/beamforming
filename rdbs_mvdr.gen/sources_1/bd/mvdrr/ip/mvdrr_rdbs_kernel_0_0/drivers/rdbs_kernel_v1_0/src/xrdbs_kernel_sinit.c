// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#ifdef SDT
#include "xparameters.h"
#endif
#include "xrdbs_kernel.h"

extern XRdbs_kernel_Config XRdbs_kernel_ConfigTable[];

#ifdef SDT
XRdbs_kernel_Config *XRdbs_kernel_LookupConfig(UINTPTR BaseAddress) {
	XRdbs_kernel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XRdbs_kernel_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XRdbs_kernel_ConfigTable[Index].Control_BaseAddress == BaseAddress) {
			ConfigPtr = &XRdbs_kernel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XRdbs_kernel_Initialize(XRdbs_kernel *InstancePtr, UINTPTR BaseAddress) {
	XRdbs_kernel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XRdbs_kernel_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XRdbs_kernel_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XRdbs_kernel_Config *XRdbs_kernel_LookupConfig(u16 DeviceId) {
	XRdbs_kernel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XRDBS_KERNEL_NUM_INSTANCES; Index++) {
		if (XRdbs_kernel_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XRdbs_kernel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XRdbs_kernel_Initialize(XRdbs_kernel *InstancePtr, u16 DeviceId) {
	XRdbs_kernel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XRdbs_kernel_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XRdbs_kernel_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

