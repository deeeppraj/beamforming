// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XMVDR_WEIGHTS_H
#define XMVDR_WEIGHTS_H

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
#include "xmvdr_weights_hw.h"

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
} XMvdr_weights_Config;
#endif

typedef struct {
    u64 Ctrl_BaseAddress;
    u32 IsReady;
} XMvdr_weights;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XMvdr_weights_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XMvdr_weights_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XMvdr_weights_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XMvdr_weights_ReadReg(BaseAddress, RegOffset) \
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
int XMvdr_weights_Initialize(XMvdr_weights *InstancePtr, UINTPTR BaseAddress);
XMvdr_weights_Config* XMvdr_weights_LookupConfig(UINTPTR BaseAddress);
#else
int XMvdr_weights_Initialize(XMvdr_weights *InstancePtr, u16 DeviceId);
XMvdr_weights_Config* XMvdr_weights_LookupConfig(u16 DeviceId);
#endif
int XMvdr_weights_CfgInitialize(XMvdr_weights *InstancePtr, XMvdr_weights_Config *ConfigPtr);
#else
int XMvdr_weights_Initialize(XMvdr_weights *InstancePtr, const char* InstanceName);
int XMvdr_weights_Release(XMvdr_weights *InstancePtr);
#endif

void XMvdr_weights_Start(XMvdr_weights *InstancePtr);
u32 XMvdr_weights_IsDone(XMvdr_weights *InstancePtr);
u32 XMvdr_weights_IsIdle(XMvdr_weights *InstancePtr);
u32 XMvdr_weights_IsReady(XMvdr_weights *InstancePtr);
void XMvdr_weights_EnableAutoRestart(XMvdr_weights *InstancePtr);
void XMvdr_weights_DisableAutoRestart(XMvdr_weights *InstancePtr);

void XMvdr_weights_Set_R_in(XMvdr_weights *InstancePtr, u64 Data);
u64 XMvdr_weights_Get_R_in(XMvdr_weights *InstancePtr);
void XMvdr_weights_Set_a_in(XMvdr_weights *InstancePtr, u64 Data);
u64 XMvdr_weights_Get_a_in(XMvdr_weights *InstancePtr);
void XMvdr_weights_Set_w_out(XMvdr_weights *InstancePtr, u64 Data);
u64 XMvdr_weights_Get_w_out(XMvdr_weights *InstancePtr);

void XMvdr_weights_InterruptGlobalEnable(XMvdr_weights *InstancePtr);
void XMvdr_weights_InterruptGlobalDisable(XMvdr_weights *InstancePtr);
void XMvdr_weights_InterruptEnable(XMvdr_weights *InstancePtr, u32 Mask);
void XMvdr_weights_InterruptDisable(XMvdr_weights *InstancePtr, u32 Mask);
void XMvdr_weights_InterruptClear(XMvdr_weights *InstancePtr, u32 Mask);
u32 XMvdr_weights_InterruptGetEnabled(XMvdr_weights *InstancePtr);
u32 XMvdr_weights_InterruptGetStatus(XMvdr_weights *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
