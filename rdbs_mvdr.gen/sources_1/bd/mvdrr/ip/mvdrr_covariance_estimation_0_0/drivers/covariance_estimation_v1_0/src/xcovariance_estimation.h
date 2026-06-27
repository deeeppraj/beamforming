// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XCOVARIANCE_ESTIMATION_H
#define XCOVARIANCE_ESTIMATION_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xcovariance_estimation_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Ctrl_BaseAddress;
} XCovariance_estimation_Config;
#endif

typedef struct {
    u64 Ctrl_BaseAddress;
    u32 IsReady;
} XCovariance_estimation;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XCovariance_estimation_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XCovariance_estimation_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XCovariance_estimation_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XCovariance_estimation_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifdef SDT
int XCovariance_estimation_Initialize(XCovariance_estimation *InstancePtr, UINTPTR BaseAddress);
XCovariance_estimation_Config* XCovariance_estimation_LookupConfig(UINTPTR BaseAddress);
#else
int XCovariance_estimation_Initialize(XCovariance_estimation *InstancePtr, u16 DeviceId);
XCovariance_estimation_Config* XCovariance_estimation_LookupConfig(u16 DeviceId);
#endif
int XCovariance_estimation_CfgInitialize(XCovariance_estimation *InstancePtr, XCovariance_estimation_Config *ConfigPtr);
#else
int XCovariance_estimation_Initialize(XCovariance_estimation *InstancePtr, const char* InstanceName);
int XCovariance_estimation_Release(XCovariance_estimation *InstancePtr);
#endif

void XCovariance_estimation_Start(XCovariance_estimation *InstancePtr);
u32 XCovariance_estimation_IsDone(XCovariance_estimation *InstancePtr);
u32 XCovariance_estimation_IsIdle(XCovariance_estimation *InstancePtr);
u32 XCovariance_estimation_IsReady(XCovariance_estimation *InstancePtr);
void XCovariance_estimation_EnableAutoRestart(XCovariance_estimation *InstancePtr);
void XCovariance_estimation_DisableAutoRestart(XCovariance_estimation *InstancePtr);

void XCovariance_estimation_Set_R(XCovariance_estimation *InstancePtr, u64 Data);
u64 XCovariance_estimation_Get_R(XCovariance_estimation *InstancePtr);

void XCovariance_estimation_InterruptGlobalEnable(XCovariance_estimation *InstancePtr);
void XCovariance_estimation_InterruptGlobalDisable(XCovariance_estimation *InstancePtr);
void XCovariance_estimation_InterruptEnable(XCovariance_estimation *InstancePtr, u32 Mask);
void XCovariance_estimation_InterruptDisable(XCovariance_estimation *InstancePtr, u32 Mask);
void XCovariance_estimation_InterruptClear(XCovariance_estimation *InstancePtr, u32 Mask);
u32 XCovariance_estimation_InterruptGetEnabled(XCovariance_estimation *InstancePtr);
u32 XCovariance_estimation_InterruptGetStatus(XCovariance_estimation *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
