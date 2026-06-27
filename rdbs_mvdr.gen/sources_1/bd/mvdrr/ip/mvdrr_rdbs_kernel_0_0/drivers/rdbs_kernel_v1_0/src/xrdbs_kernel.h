// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XRDBS_KERNEL_H
#define XRDBS_KERNEL_H

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
#include "xrdbs_kernel_hw.h"

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
    u64 Control_BaseAddress;
} XRdbs_kernel_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XRdbs_kernel;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XRdbs_kernel_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XRdbs_kernel_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XRdbs_kernel_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XRdbs_kernel_ReadReg(BaseAddress, RegOffset) \
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
int XRdbs_kernel_Initialize(XRdbs_kernel *InstancePtr, UINTPTR BaseAddress);
XRdbs_kernel_Config* XRdbs_kernel_LookupConfig(UINTPTR BaseAddress);
#else
int XRdbs_kernel_Initialize(XRdbs_kernel *InstancePtr, u16 DeviceId);
XRdbs_kernel_Config* XRdbs_kernel_LookupConfig(u16 DeviceId);
#endif
int XRdbs_kernel_CfgInitialize(XRdbs_kernel *InstancePtr, XRdbs_kernel_Config *ConfigPtr);
#else
int XRdbs_kernel_Initialize(XRdbs_kernel *InstancePtr, const char* InstanceName);
int XRdbs_kernel_Release(XRdbs_kernel *InstancePtr);
#endif

void XRdbs_kernel_Start(XRdbs_kernel *InstancePtr);
u32 XRdbs_kernel_IsDone(XRdbs_kernel *InstancePtr);
u32 XRdbs_kernel_IsIdle(XRdbs_kernel *InstancePtr);
u32 XRdbs_kernel_IsReady(XRdbs_kernel *InstancePtr);
void XRdbs_kernel_EnableAutoRestart(XRdbs_kernel *InstancePtr);
void XRdbs_kernel_DisableAutoRestart(XRdbs_kernel *InstancePtr);

void XRdbs_kernel_Set_num_snapshots(XRdbs_kernel *InstancePtr, u32 Data);
u32 XRdbs_kernel_Get_num_snapshots(XRdbs_kernel *InstancePtr);

void XRdbs_kernel_InterruptGlobalEnable(XRdbs_kernel *InstancePtr);
void XRdbs_kernel_InterruptGlobalDisable(XRdbs_kernel *InstancePtr);
void XRdbs_kernel_InterruptEnable(XRdbs_kernel *InstancePtr, u32 Mask);
void XRdbs_kernel_InterruptDisable(XRdbs_kernel *InstancePtr, u32 Mask);
void XRdbs_kernel_InterruptClear(XRdbs_kernel *InstancePtr, u32 Mask);
u32 XRdbs_kernel_InterruptGetEnabled(XRdbs_kernel *InstancePtr);
u32 XRdbs_kernel_InterruptGetStatus(XRdbs_kernel *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
