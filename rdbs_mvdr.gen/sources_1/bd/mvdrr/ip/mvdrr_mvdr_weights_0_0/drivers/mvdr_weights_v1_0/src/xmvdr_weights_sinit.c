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
#include "xmvdr_weights.h"

extern XMvdr_weights_Config XMvdr_weights_ConfigTable[];

#ifdef SDT
XMvdr_weights_Config *XMvdr_weights_LookupConfig(UINTPTR BaseAddress) {
	XMvdr_weights_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XMvdr_weights_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XMvdr_weights_ConfigTable[Index].Ctrl_BaseAddress == BaseAddress) {
			ConfigPtr = &XMvdr_weights_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XMvdr_weights_Initialize(XMvdr_weights *InstancePtr, UINTPTR BaseAddress) {
	XMvdr_weights_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XMvdr_weights_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XMvdr_weights_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XMvdr_weights_Config *XMvdr_weights_LookupConfig(u16 DeviceId) {
	XMvdr_weights_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XMVDR_WEIGHTS_NUM_INSTANCES; Index++) {
		if (XMvdr_weights_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XMvdr_weights_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XMvdr_weights_Initialize(XMvdr_weights *InstancePtr, u16 DeviceId) {
	XMvdr_weights_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XMvdr_weights_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XMvdr_weights_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

