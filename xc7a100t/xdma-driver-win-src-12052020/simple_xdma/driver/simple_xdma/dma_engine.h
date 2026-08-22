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
-- Revision       : $Revision: #10 $
-- Date           : $DateTime: 2019/06/30 21:08:14 $
-- Last Author    : $Author: arayajig $
--
-------------------------------------------------------------------------------
-- Description :
-- This file is part of the Xilinx DMA IP Core driver for Windows.
--
-------------------------------------------------------------------------------
*/

#pragma once

// ========================= include dependencies =================================================

#include <ntddk.h>
#include <ntintsafe.h>
#include <wdf.h>
#include "reg.h"
#include "xdma_public.h"

// ========================= constants ============================================================

#define XDMA_MAX_NUM_CHANNELS   (4)
#define XDMA_NUM_DIRECTIONS     (2)
#define XDMA_MAX_CHAN_IRQ       (XDMA_NUM_DIRECTIONS * XDMA_MAX_NUM_CHANNELS)
#define XDMA_MAX_TRANSFER_SIZE  (8UL * 1024UL * 1024UL)

// ========================= forward declarations =================================================

struct XDMA_DEVICE_T; 
typedef struct XDMA_DEVICE_T XDMA_DEVICE;
struct XDMA_ENGINE_T;

// ========================= type declarations ====================================================

/// Direction of the DMA transfer/engine
typedef enum DirToDev_t {
    H2C = 0, // Host-to-Card - write to device
    C2H = 1  // Card-to-Host - read from device
} DirToDev;

/// engine specific work to perform after dma transfer completion is detected
typedef VOID(*PFN_XDMA_ENGINE_WORK)(IN struct XDMA_ENGINE_T *engine);

/// Engine address mode. 
/// Determines how the DMA engine interprets the device address (destination address on H2C and 
/// source address on C2H).
/// When AddressMode_Contiguous is chosen, the device address only needs to be set for the first 
/// descriptor and the engine assumes that all subsequent descriptors device addresses follow
/// sequentially. 
/// When AddressMode_Fixed is selected, the device addresses of each descriptor must be explicitly
/// set.
typedef enum AddressMode_T {
    AddressMode_Contiguous,  // incremental
    AddressMode_Fixed,       // non-incremental
} AddressMode;

typedef enum EngineType_t {
    EngineType_MM,      // Memory Mapped
    EngineType_ST,      // Streaming
} EngineType;

/// DMA engine abstraction
typedef struct XDMA_ENGINE_T {

    XDMA_DEVICE *parentDevice; // the xdma device to which this engine belongs

    // register access
    volatile XDMA_ENGINE_REGS *regs; // control regs
    volatile XDMA_SGDMA_REGS *sgdma;

    // engine configuration
    UINT32 irqBitMask;
    UINT32 alignAddr;
    UINT32 alignLength;
    UINT32 alignAddrBits;
    DWORD channel;
    DirToDev dir;               // data flow direction (H2C or C2H)
    BOOLEAN enabled;
    EngineType type;            // MemoryMapped or Streaming
    AddressMode addressMode;    // incremental (contiguous) or non-incremental (fixed)

    // for engine request tracking
    UINT32 capacity;
    volatile BOOLEAN isReqPending;
    WDFSPINLOCK engineLock;

    // dma transfer related
    WDFCOMMONBUFFER descBuffer;
    WDFDMATRANSACTION dmaTransaction;
    PFN_XDMA_ENGINE_WORK work; // engine work for interrupt processing

    // watchdog timer for DMA transfer timeout
    KTIMER watchdogTimer;
    KDPC watchdogDpc;
} XDMA_ENGINE;

#pragma pack(1)

/// \brief Descriptor for a single contiguous memory block transfer.
///
/// Multiple descriptors are linked a 'next' pointer. An additional extra adjacent number gives the 
/// amount of subsequent contiguous descriptors. The descriptors are in root complex memory, and the
/// bytes in the 32-bit words must be in little-endian byte ordering.
typedef struct xdma_descriptor_t {
    UINT32 control;
    UINT32 numBytes;  // transfer length in bytes
    UINT32 srcAddrLo; // source address (low 32-bit)
    UINT32 srcAddrHi; // source address (high 32-bit)
    UINT32 dstAddrLo; // destination address (low 32-bit)
    UINT32 dstAddrHi; // destination address (high 32-bit)
                      // next descriptor in the single-linked list of descriptors, 
                      // this is the bus address of the next descriptor in the root complex memory.
    UINT32 nextLo;    // next desc address (low 32-bit)
    UINT32 nextHi;    // next desc address (high 32-bit)
} DMA_DESCRIPTOR;

#pragma pack()

// ========================= function declarations ================================================

struct XDMA_DEVICE_T;
typedef struct XDMA_DEVICE_T* PXDMA_DEVICE;

/// Queries the existence of engine
void CountChannels(IN PXDMA_DEVICE xdma, OUT ULONG *h2cCount, OUT ULONG *c2hCount);

/// Initialize an XDMA_ENGINE for each engine configured in HW
NTSTATUS ProbeEngines(IN PXDMA_DEVICE xdma);

/// Start the DMA engine
/// The transfer descriptors should be initialized and bound to HW before calling this function
VOID EngineStart(IN XDMA_ENGINE *engine);

/// Stop the DMA engine
VOID EngineStop(IN XDMA_ENGINE *engine);

/// enable the engines interrupt
VOID EngineEnableInterrupt(IN XDMA_ENGINE* engine);

/// disable the engines interrupt
VOID EngineDisableInterrupt(IN XDMA_ENGINE* engine);

/// Stringify the Engine direction (H2C/C2H)
char* DirectionToString(DirToDev dir);