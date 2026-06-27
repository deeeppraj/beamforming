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
#include "xcovariance_estimation.h"

extern XCovariance_estimation_Config XCovariance_estimation_ConfigTable[];

#ifdef SDT
XCovariance_estimation_Config *XCovariance_estimation_LookupConfig(UINTPTR BaseAddress) {
	XCovariance_estimation_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XCovariance_estimation_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XCovariance_estimation_ConfigTable[Index].Ctrl_BaseAddress == BaseAddress) {
			ConfigPtr = &XCovariance_estimation_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XCovariance_estimation_Initialize(XCovariance_estimation *InstancePtr, UINTPTR BaseAddress) {
	XCovariance_estimation_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XCovariance_estimation_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XCovariance_estimation_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XCovariance_estimation_Config *XCovariance_estimation_LookupConfig(u16 DeviceId) {
	XCovariance_estimation_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XCOVARIANCE_ESTIMATION_NUM_INSTANCES; Index++) {
		if (XCovariance_estimation_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XCovariance_estimation_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XCovariance_estimation_Initialize(XCovariance_estimation *InstancePtr, u16 DeviceId) {
	XCovariance_estimation_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XCovariance_estimation_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XCovariance_estimation_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

