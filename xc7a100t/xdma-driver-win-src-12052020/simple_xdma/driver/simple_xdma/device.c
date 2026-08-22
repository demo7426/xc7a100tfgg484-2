/*
-- (c) Copyright 2019 Xilinx, Inc. All rights reserved.
--
-- This file contains confidential and proprietary information
-- of Xilinx, Inc. and is protected under U.S. and
-- international copyright and other intellectual property
-- laws.
--
-- DISCLAIMER
-- This disclaimer is not a license and does not grant any
-- rights to the materials distributed herewith. Except as
-- otherwise provided in a Valid license issued to you by
-- Xilinx, and to the maximum extent permitted by applicable
-- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
-- WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
-- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
-- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
-- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
-- (2) Xilinx shall not be liable (whether in contract or tort,
-- including negligence, or under any other theory of
-- liability) for any loss or damage of any kind or nature
-- related to, arising under or in connection with these
-- materials, including for any direct, or any indirect,
-- special, incidental, or consequential loss or damage
-- (including loss of Data, profits, goodwill, or any type of
-- loss or damage suffered as a result of any action brought
-- by a third party) even if such damage or loss was
-- reasonably foreseeable or Xilinx had been advised of the
-- possibility of the same.
--
-- CRITICAL APPLICATIONS
-- Xilinx products are not designed or intended to be fail-
-- safe, or for use in any application requiring fail-safe
-- performance, such as life-support or safety devices or
-- systems, Class III medical devices, nuclear facilities,
-- applications related to the deployment of airbags, or any
-- other applications that could lead to death, personal
-- injury, or severe property or environmental damage
-- (individually and collectively, "Critical
-- Applications"). Customer assumes the sole risk and
-- liability of any use of Xilinx products in Critical
-- Applications, subject only to applicable laws and
-- regulations governing limitations on product liability.
--
-- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
-- PART OF THIS FILE AT ALL TIMES.
-------------------------------------------------------------------------------
--
-- Vendor         : Xilinx
-- Revision       : $Revision: #9 $
-- Date           : $DateTime: 2019/06/30 21:08:14 $
-- Last Author    : $Author: arayajig $
--
-------------------------------------------------------------------------------
-- Description :
-- This file is part of the Xilinx DMA IP Core driver for Windows.
--
-------------------------------------------------------------------------------
*/

// ====================== include dependancies ========================================================

#include <ntddk.h>
#include <initguid.h> // required for GUID definitions
#include <wdmguid.h> // required for WMILIB_CONTEXT

#include "device.h"
#include "interrupt.h"
#include "dma_engine.h"
#include "xdma_public.h"

#include "trace.h"
#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro 
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "device.tmh"
#endif

// ====================== Function declarations ========================================================

// declare following functions as pageable code
#ifdef ALLOC_PRAGMA

#endif

// WDK 10 static code analysis feature expects to target Windows 10 and thus recommends not to
// use MmMapIoSpace and use MmMapIoSpaceEx instead. However this function is not available pre 
// Win 10. Thus disable this warning. 
// see https://social.msdn.microsoft.com/Forums/en-US/f8a3fb63-10de-481c-b629-8b5f3d254c5e/unexpected-code-analysis-behavior?forum=wdk
#pragma warning (disable : 30029) 

// ====================== constants ========================================================

// Version constants for the XMDA IP core
typedef enum XDMA_IP_VERSION_T {
    v2015_4 = 1,
    v2016_1 = 2,
    v2016_2 = 3,
    v2016_3 = 4,
    v2016_4 = 5,
    v2017_1 = 6,
    v2017_2 = 7,
    v2017_3 = 8
} XDMA_IP_VERSION;

// ====================== static functions ========================================================

// Initialize the XDMA_DEVICE structure with default values
static void DeviceDefaultInitialize(IN PXDMA_DEVICE xdma) {
    ASSERT(xdma != NULL);

    // bars
    xdma->numBars = 0;
    for (UINT32 i = 0; i < XDMA_MAX_NUM_BARS; i++) {
        xdma->bar[i] = NULL;
        xdma->barLength[i] = 0;
    }
    xdma->configBarIdx = 0;
    xdma->userBarIdx = -1;
    xdma->bypassBarIdx = -1;

    // registers 
    xdma->configRegs = NULL;
    xdma->interruptRegs = NULL;
    xdma->sgdmaRegs = NULL;

    // engines
    for (UINT dir = H2C; dir < 2; dir++) { // 0=H2C, 1=C2H
        for (ULONG ch = 0; ch < XDMA_MAX_NUM_CHANNELS; ch++) {
            xdma->engines[ch][dir].enabled = FALSE;
            xdma->engines[ch][dir].poll = FALSE;
        }
    }

    // interrupts - nothing to do

    // user events
    for (int i = 0; i < XDMA_MAX_USER_IRQ; i++) {
        xdma->userEvents[i].work = NULL;
        xdma->userEvents[i].userData = NULL;
    }
}

// 读取 PCI 配置空间的 BAR0..BAR5，按升序输出「已实现的 memory BAR」的真实物理编号。
// 资源列表里第 N 个 CmResourceTypeMemory 即对应 realBarIndices[N]。
static NTSTATUS GetRealBarIndices(IN WDFDEVICE wdfDevice,
    OUT ULONG realBarIndices[PCI_TYPE0_ADDRESSES],
    OUT PULONG numRealBars) {
    *numRealBars = 0;

    BUS_INTERFACE_STANDARD pciBus = { 0 };
    NTSTATUS status = WdfFdoQueryForInterface(wdfDevice, &GUID_BUS_INTERFACE_STANDARD,
        (PINTERFACE)&pciBus, sizeof(BUS_INTERFACE_STANDARD), 1, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    PCI_COMMON_HEADER pciHeader = { 0 };
    ULONG bytesRead = pciBus.GetBusData(pciBus.Context, PCI_WHICHSPACE_CONFIG,
        &pciHeader, 0, PCI_COMMON_HDR_LENGTH);
    if (bytesRead != (ULONG)PCI_COMMON_HDR_LENGTH) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    // BAR0..BAR5 位于 u.type0.BaseAddresses[0..5]
    for (ULONG i = 0; i < PCI_TYPE0_ADDRESSES; i++) {
        ULONG barVal = pciHeader.u.type0.BaseAddresses[i];
        if (barVal == 0) {
            continue;                          // 未实现
        }
        if (barVal & 0x1) {
            continue;                          // I/O BAR，跳过
        }
        BOOLEAN is64bit = (((barVal >> 1) & 0x3) == 0x2); // bits[2:1]==10
        BOOLEAN prefetchable = (barVal & 0x8) ? TRUE : FALSE;

        realBarIndices[(*numRealBars)++] = i;

        if (is64bit && i + 1 < PCI_TYPE0_ADDRESSES) {
            ULONGLONG barRaw = (ULONGLONG)barVal | ((ULONGLONG)pciHeader.u.type0.BaseAddresses[i + 1] << 32);

            TraceInfo(DBG_INIT, "%s, bar[%u] raw=0x%llx, addr=0x%llx, 64-bit%s",
                __func__, i, barRaw, barRaw & 0xFFFFFFFFFFFFFFF0,
                prefetchable ? ", prefetchable" : ", non-prefetchable");

            i++;                               // 高 32 位占用下一槽
        }
        else
        {
            TraceInfo(DBG_INIT, "%s, bar[%u] raw=0x%x, addr=0x%x, 32-bit%s",
                __func__, i, barVal, barVal & 0xFFFFFFF0,
                prefetchable ? ", prefetchable" : ", non-prefetchable");

        }
    }
    return STATUS_SUCCESS;
}

// Iterate through PCIe resources and map BARS into host memory
static NTSTATUS MapBARs(IN PXDMA_DEVICE xdma, IN WDFCMRESLIST ResourcesTranslated) {
    const ULONG numResources = WdfCmResourceListGetCount(ResourcesTranslated);
    TraceVerbose(DBG_INIT, "# PCIe resources = %d", numResources);

#if 1
    // 先从 PCI 配置空间恢复真实 BAR 编号
    ULONG realBarIndices[PCI_TYPE0_ADDRESSES] = { 0 };
    ULONG numRealBars = 0;
    NTSTATUS status = GetRealBarIndices(xdma->wdfDevice, realBarIndices, &numRealBars);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "GetRealBarIndices() failed: %!STATUS!", status);
        return status;
    }
#endif

    for (UINT i = 0; i < numResources; i++) {
        PCM_PARTIAL_RESOURCE_DESCRIPTOR resource = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);
        if (!resource) {
            TraceError(DBG_INIT, "WdfResourceCmGetDescriptor() fails");
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }

        switch (resource->Type)
        {
        case CmResourceTypePort:
            break;
        case CmResourceTypeInterrupt:
            break;
        case CmResourceTypeDma:
            break;
        case CmResourceTypeMemory:
        {
            xdma->barLength[xdma->numBars] = resource->u.Memory.Length;

            xdma->bar[xdma->numBars] = MmMapIoSpace(resource->u.Memory.Start,
                resource->u.Memory.Length, MmNonCached);
            if (xdma->bar[xdma->numBars] == NULL) {
                TraceError(DBG_INIT, "MmMapIoSpace returned NULL! for BAR%u", xdma->numBars);
                return STATUS_DEVICE_CONFIGURATION_ERROR;
            }

            TraceInfo(DBG_INIT, "MM BAR %d (addr:0x%llx, length:0x%x) mapped at 0x%08p",
                xdma->numBars, resource->u.Memory.Start.QuadPart,
                resource->u.Memory.Length, xdma->bar[xdma->numBars]);
            
            xdma->numBars++;

            TraceInfo(DBG_INIT, "%s, CmResourceTypeMemory: numBars = %u", __func__, xdma->numBars);
        }
        break;
        case CmResourceTypeMemoryLarge: //resource->u.Memory.Length 长度 >4GB 才触发
            break;
        default:
            break;
        }

    }
    return STATUS_SUCCESS;
}

// Is the BAR at index 'idx' the config BAR?
static BOOLEAN IsConfigBAR(IN PXDMA_DEVICE xdma, IN UINT idx) {

    XDMA_IRQ_REGS* irqRegs = (XDMA_IRQ_REGS*)((PUCHAR)xdma->bar[idx] + IRQ_BLOCK_OFFSET);
    XDMA_CONFIG_REGS* cfgRegs = (XDMA_CONFIG_REGS*)((PUCHAR)xdma->bar[idx] + CONFIG_BLOCK_OFFSET);

    UINT32 interruptID = irqRegs->identifier & XDMA_ID_MASK;
    UINT32 configID = cfgRegs->identifier & XDMA_ID_MASK;

    return ((interruptID == XDMA_ID) && (configID == XDMA_ID)) ? TRUE : FALSE;
}

// Get the config, interrupt and sgdma module register offsets
static void GetRegisterModules(IN PXDMA_DEVICE xdma) {
    PUCHAR configBarAddr = (PUCHAR)xdma->bar[xdma->configBarIdx];
    xdma->configRegs = (XDMA_CONFIG_REGS*)(configBarAddr + CONFIG_BLOCK_OFFSET);
    xdma->interruptRegs = (XDMA_IRQ_REGS*)(configBarAddr + IRQ_BLOCK_OFFSET);
    xdma->sgdmaRegs = (XDMA_SGDMA_COMMON_REGS*)(configBarAddr + SGDMA_COMMON_BLOCK_OFFSET);
}

// ====================== API functions ========================================

NTSTATUS XDMA_DeviceOpen(WDFDEVICE wdfDevice,
                         PXDMA_DEVICE xdma,
                         ULONG *userMax,
                         ULONG *h2cChannelMax,
                         ULONG *c2hChannelMax,
                         WDFCMRESLIST ResourcesRaw,
                         WDFCMRESLIST ResourcesTranslated
                         ) {

    NTSTATUS status = STATUS_INTERNAL_ERROR;

    if (NULL == xdma)
        return STATUS_INVALID_PARAMETER;

    DeviceDefaultInitialize(xdma);

    xdma->wdfDevice = wdfDevice;

    // map PCIe BARs to host memory
    status = MapBARs(xdma, ResourcesTranslated);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "MapBARs() failed! %!STATUS!", status);
        return status;
    }

    xdma->configBarIdx = 1;
    xdma->userBarIdx = -1;
    xdma->bypassBarIdx = -1;

    // get the module offsets in config BAR
    GetRegisterModules(xdma);

    status = SetupInterrupts(xdma, userMax, h2cChannelMax, c2hChannelMax, ResourcesRaw, ResourcesTranslated);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "SetupInterrupts failed: %!STATUS!", status);
        return status;
    }

    // WDF DMA Enabler - at least 8 bytes alignment
    WdfDeviceSetAlignmentRequirement(xdma->wdfDevice, 8 - 1); // TODO - choose correct value
    WDF_DMA_ENABLER_CONFIG dmaConfig;
    WDF_DMA_ENABLER_CONFIG_INIT(&dmaConfig, WdfDmaProfileScatterGather64Duplex, XDMA_MAX_TRANSFER_SIZE);
    status = WdfDmaEnablerCreate(xdma->wdfDevice, &dmaConfig, WDF_NO_OBJECT_ATTRIBUTES, &xdma->dmaEnabler);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, " WdfDmaEnablerCreate() failed: %!STATUS!", status);
        return status;
    }

    // Detect and initialize engines configured in HW IP 
    status = ProbeEngines(xdma);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "ProbeEngines failed: %!STATUS!", status);
        return status;
    }

    return status;
}

void XDMA_DeviceClose(PXDMA_DEVICE xdma) {

    // todo - stop every engine?

    // reset irq vectors?
    if (xdma && xdma->interruptRegs) {
        xdma->interruptRegs->userVector[0] = 0;
        xdma->interruptRegs->userVector[1] = 0;
        xdma->interruptRegs->userVector[2] = 0;
        xdma->interruptRegs->userVector[3] = 0;
        xdma->interruptRegs->channelVector[0] = 0;
        xdma->interruptRegs->channelVector[1] = 0;
    }

    closeEngines(xdma);

    // Unmap any I/O ports. Disconnecting from the interrupt will be done automatically by the framework.
    for (UINT i = 0; i < xdma->numBars; i++) {
        if (xdma->bar[i] != NULL) {
            TraceInfo(DBG_INIT, "Unmapping BAR%d, VA:(%p) Length %ul",
                      i, xdma->bar[i], xdma->barLength[i]);
            MmUnmapIoSpace(xdma->bar[i], xdma->barLength[i]);
            xdma->bar[i] = NULL;
        }
    }
}



