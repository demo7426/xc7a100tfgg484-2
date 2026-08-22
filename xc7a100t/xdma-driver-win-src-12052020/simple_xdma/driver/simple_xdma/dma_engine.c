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
-- related to, arising under, in connection with these
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
-- Revision       : $Revision: #12 $
-- Date           : $DateTime: 2019/06/30 21:08:14 $
-- Last Author    : $Author: arayajig $
--
-------------------------------------------------------------------------------
-- Description :
-- This file is part of the Xilinx DMA IP Core driver for Windows.
-- Simplified version: MM engines only, interrupt-driven completion with
-- KTIMER-based watchdog timeout.
--
-------------------------------------------------------------------------------
*/

// ========================= include dependencies =================================================

#include "device.h"
#include "interrupt.h"
#include "dma_engine.h"
#include "trace.h"

#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro 
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "dma_engine.tmh"
#endif

// ========================= constants ============================================================

#define XDMA_ENG_IRQ_NUM        (1)
#define XDMA_DESC_MAGIC         (0xAD4B0000)

// 10 seconds watchdog timeout for DMA transfer completion
#define XDMA_DMA_TIMEOUT_SECONDS    (10)
#define XDMA_DMA_TIMEOUT_100NS      ((LONGLONG)(-XDMA_DMA_TIMEOUT_SECONDS) * 10000000LL)

// ========================= static function declarations =========================================

static UINT32 EngineStatus(IN XDMA_ENGINE *engine, IN BOOLEAN clear);
static void EngineGetAlignments(IN OUT XDMA_ENGINE *engine);
static NTSTATUS EngineCreateDescriptorBuffer(IN OUT XDMA_ENGINE *engine);
static void EngineConfigureInterrupt(IN OUT XDMA_ENGINE *engine, IN UINT index, IN ULONG engineId);
static void EngineProcessTransfer(IN XDMA_ENGINE *engine);
static VOID EngineWatchdogDpc(IN PKDPC Dpc, IN PVOID context, IN PVOID arg1, IN PVOID arg2);

// Mark these functions as pageable code
#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, ProbeEngines)
#endif

// WDK 10 static code analysis gives a false warning: "Allocating executable memory via specifying 
// a MM_PAGE_PRIORITY type without a bitwise OR with MdlMappingNoExecute"
// However this only applies to Windows 8 and above, thus disable this warning. 
// see https://msdn.microsoft.com/en-us/library/windows/hardware/ff554629(v=vs.85).aspx
#pragma warning (disable : 30030) 

// ======================== common engine functions ===============================================
static BOOLEAN isReqPending(IN XDMA_ENGINE *engine)
{
    BOOLEAN isReqPending;

    WdfSpinLockAcquire(engine->engineLock);
    isReqPending = engine->isReqPending;
    WdfSpinLockRelease(engine->engineLock);

    return isReqPending;
}

static VOID markReqPending(IN XDMA_ENGINE *engine)
{
    WdfSpinLockAcquire(engine->engineLock);
    engine->isReqPending = TRUE;
    WdfSpinLockRelease(engine->engineLock);
}

static NTSTATUS EngineCreateDescriptorBuffer(IN OUT XDMA_ENGINE *engine) {
    // allocate host-side buffer for descriptors
    SIZE_T bufferSize = (XDMA_MAX_TRANSFER_SIZE / PAGE_SIZE + 2) * sizeof(DMA_DESCRIPTOR);

    NTSTATUS status = WdfCommonBufferCreate(engine->parentDevice->dmaEnabler, bufferSize,
                                            WDF_NO_OBJECT_ATTRIBUTES, &engine->descBuffer);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "WdfCommonBufferCreate failed: %!STATUS!", status);
        return status;
    }

    engine->capacity = (UINT32)(bufferSize / sizeof(DMA_DESCRIPTOR));

    PHYSICAL_ADDRESS descBufferLA = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);
    PUCHAR descBufferVA = (PUCHAR)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
    RtlZeroMemory(descBufferVA, bufferSize);

    // give hw the physical start address of the descriptor buffer
    engine->sgdma->firstDescLo = descBufferLA.LowPart;
    engine->sgdma->firstDescHi = descBufferLA.HighPart;
    engine->sgdma->firstDescAdj = 0; // depends on transfer - set later in ProgramDMA

    TraceVerbose(DBG_INIT, "descriptor buffer at 0x%08x%08x, size=%lld, capacity=%u",
                 engine->sgdma->firstDescHi, engine->sgdma->firstDescLo,
                 bufferSize, engine->capacity);

    return status;
}

static void EngineConfigureInterrupt(IN OUT XDMA_ENGINE *engine, IN UINT index, IN ULONG engineId) {
    // engine interrupt request bit(s) - interrupt bit depends on number of engines present
    // see Figure 2-4 on page 46 of pcie dma product guide [1]
    engine->irqBitMask = (1 << XDMA_ENG_IRQ_NUM) - 1;
    engine->irqBitMask <<= (engineId * XDMA_ENG_IRQ_NUM);

    // bind msi interrupt context with this engine
    if (engine->parentDevice->channelInterrupts[index] != NULL) {
        PIRQ_CONTEXT irqContext = GetIrqContext(engine->parentDevice->channelInterrupts[index]);
        irqContext->engine = engine;
    }

    // enable interrupts
    UINT32 regVal = XDMA_CTRL_IE_ALL;
    engine->regs->intEnableMaskW1S = regVal;
    engine->regs->controlW1S = regVal;
    TraceVerbose(DBG_INIT, "engineIrqBitMask=0x%08x, intEnableMask=0x%08x",
                 engine->irqBitMask, engine->regs->intEnableMask);
}

static void EngineProcessTransfer(IN XDMA_ENGINE *engine)
// service an SGDMA engine
{
    NTSTATUS status = STATUS_SUCCESS;
    WDFREQUEST request;
    UINT32 engineStatus;
    BOOLEAN completed;

    if (engine == NULL) {
        TraceError(DBG_DMA, "engine=NULL");
        return;
    }

    TraceInfo(DBG_DMA, "%s_%u processing transfer completion",
              DirectionToString(engine->dir), engine->channel);

    WdfSpinLockAcquire(engine->engineLock);

    if (FALSE == engine->isReqPending) {
        /* Spurious interrupt ?? */
        TraceError(DBG_DMA, "No request submitted for engine.");
        WdfSpinLockRelease(engine->engineLock);
        return;
    }

    request = WdfDmaTransactionGetRequest(engine->dmaTransaction);
    if (!request) {
        TraceInfo(DBG_DMA, "Interrupt but no request pending?");
        WdfSpinLockRelease(engine->engineLock);
        return;
    }

    // cancel the watchdog timer - the DMA completed normally
    KeCancelTimer(&engine->watchdogTimer);

    // read and clear engine status 
    engineStatus = EngineStatus(engine, TRUE);

    EngineStop(engine);

    // clear descriptor buffer
    DMA_DESCRIPTOR* descriptorBuffer = (DMA_DESCRIPTOR*)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
    size_t descBufferLength = WdfCommonBufferGetLength(engine->descBuffer);
    RtlZeroMemory(descriptorBuffer, descBufferLength);

    // If any data is pending then call to WdfDmaTransactionDmaCompleted will immediately result in
    // another call to XDMA_EngineProgramDma. To avoid deadlock, mark the engine as free and release the lock.
    engine->isReqPending = FALSE;
    // reenable interrupt for this dma engine
    EngineEnableInterrupt(engine);
    WdfSpinLockRelease(engine->engineLock);

    switch (engineStatus & XDMA_STAT_EXPECTED_ZERO) {
    case XDMA_ENGINE_STOPPED_OK: // engine not busy and no errors?
    {
        completed = WdfDmaTransactionDmaCompleted(engine->dmaTransaction, &status);
        size_t bytesTransferred = WdfDmaTransactionGetBytesTransferred(engine->dmaTransaction);

        TraceInfo(DBG_DMA, "%s_%u transaction%scomplete, bytesTransferred=%llu",
                  DirectionToString(engine->dir), engine->channel,
                  completed ? " " : " in", bytesTransferred);

        if (completed) {
            status = WdfDmaTransactionRelease(engine->dmaTransaction);
            if (!NT_SUCCESS(status)) {
                TraceError(DBG_DMA, "WdfDmaTransactionRelease failed: %!STATUS!", status);
            }

            WdfRequestCompleteWithInformation(request, status, bytesTransferred);
        }
        break;
    }
    case XDMA_BUSY_BIT: // engine is still busy without sign of errors?
        TraceError(DBG_DMA, "Engine Still Busy, Descriptors Completed=%u",
                   engine->regs->completedDescCount);
    default: // any sign of errors
        TraceError(DBG_DMA, "Unexpected engine status 0x%08x", engineStatus);

        completed = WdfDmaTransactionDmaCompletedFinal(engine->dmaTransaction, 0, &status);
        if (completed) {
            status = WdfDmaTransactionRelease(engine->dmaTransaction);
            if (!NT_SUCCESS(status)) {
                TraceError(DBG_DMA, "WdfDmaTransactionRelease failed: %!STATUS!", status);
            }

            WdfRequestComplete(request, STATUS_INTERNAL_ERROR);
        }
    }
}

// Watchdog DPC: fires if a DMA transfer does not complete within XDMA_DMA_TIMEOUT_SECONDS.
// Cleans up the pending request and completes it with STATUS_TIMEOUT.
static VOID EngineWatchdogDpc(IN PKDPC Dpc, IN PVOID context, IN PVOID arg1, IN PVOID arg2) {
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(arg1);
    UNREFERENCED_PARAMETER(arg2);

    XDMA_ENGINE* engine = (XDMA_ENGINE*)context;
    if (engine == NULL) {
        return;
    }

    WdfSpinLockAcquire(engine->engineLock);
    if (FALSE == engine->isReqPending) {
        // Already cleaned up by the normal completion path (KeCancelTimer ran first)
        WdfSpinLockRelease(engine->engineLock);
        return;
    }
    engine->isReqPending = FALSE;
    (void)EngineStatus(engine, TRUE);
    EngineStop(engine);
    WdfSpinLockRelease(engine->engineLock);

    WDFREQUEST request = WdfDmaTransactionGetRequest(engine->dmaTransaction);
    if (request) {
        TraceError(DBG_DMA, "%s_%u watchdog timeout",
                   DirectionToString(engine->dir), engine->channel);
        WdfDmaTransactionRelease(engine->dmaTransaction);
        WdfRequestComplete(request, STATUS_TIMEOUT);
    }
}

static void DumpDescriptor(IN const DMA_DESCRIPTOR* const desc) {
#if DBG
    TraceVerbose(DBG_DESC, "descriptor={.control=0x%08X, .numBytes=%u, srcAddr=0x%08X%08X, .dstAddr=0x%08X%08X, nextAddr=0x%08X%08X}",
                 desc->control, desc->numBytes,
                 desc->srcAddrHi, desc->srcAddrLo,
                 desc->dstAddrHi, desc->dstAddrLo,
                 desc->nextHi, desc->nextLo);
#else 
    UNREFERENCED_PARAMETER(desc);
#endif
}

static BOOLEAN DescriptorIsAligned(IN XDMA_ENGINE *engine, IN DMA_DESCRIPTOR *desc)
// For alignment requirements see product guide [1] page 23 table 2-9
{
    if (engine->addressMode == AddressMode_Fixed) {
        const UINT32 dataPathWidth = (1 << (6 + engine->parentDevice->configRegs->pcieWidth)) / 8;
        const UINT32 addrMask = dataPathWidth - 1;

        if ((desc->dstAddrLo & addrMask) != (desc->srcAddrLo & addrMask) != 0) {
            TraceError(DBG_DESC, "misalignment detected! dataWidth=%u, src=0x%08llx, dst=0x%08llx, bytes=%llu",
                       dataPathWidth, desc->srcAddrLo, desc->dstAddrLo, desc->numBytes);
            return FALSE;
        }
        // todo length alignment requirement??

    } else { // AddressMode_Contiguous (i.e. incremental mode)
        BOOLEAN dstAddrMisaligned = (desc->dstAddrLo % engine->alignAddr) != 0;
        BOOLEAN lengthMisaligned = (desc->numBytes % engine->alignLength) != 0;
        BOOLEAN srcAddrMisaligned = (desc->srcAddrLo % engine->alignAddr) != 0;

        if (dstAddrMisaligned || lengthMisaligned || srcAddrMisaligned) {
            TraceVerbose(DBG_DESC, "descriptor alignments: src=0x%08llx, dst=0x%08llx, bytes=%llu",
                         desc->srcAddrLo, desc->dstAddrLo, desc->numBytes);
            TraceError(DBG_DESC, "misalignment detected! src?=%u dst?=%u length?=%u",
                       srcAddrMisaligned, dstAddrMisaligned, lengthMisaligned);
            return FALSE;
        }
    }
    return TRUE;
}

static void OptimizeDescriptors(IN XDMA_ENGINE *engine, IN DMA_DESCRIPTOR * const desc,
                                IN const ULONG numDesc)
    // Optimize descriptors for PCIe block fetches.
    // Multiple descriptors which reside in host memory can be fetched in a single PCIe transaction
    // by the device. This is achieved as follows:
    //      - For the first fetch, the number of additional (adjacent) descriptors to fetch is 
    //        specified by writing to engine->sgdma->firstDescAdj register. 
    //      - For subsequent fetches, the last descriptor of the previous fetch specifies the number of
    //        additional (adjacent) descriptors in the control->nextAdj field 
    // There are several factors which limit the amount of descriptors which can be fetched together:
    //      1. The PCIe Max Read Request Size
    //      2. The physical address of the descriptors within a block must not cross a 4K address 
    //         boundary
    //      3. The number of descriptors remaining in the transfer
{
    const ULONG mrrsBytes = 1 << (engine->parentDevice->configRegs->pcieMRRS + 7);
    const ULONG adjMax = mrrsBytes / sizeof(DMA_DESCRIPTOR) - 1;
    const ULONG adjTotal = numDesc - 1;
    const ULONG adjTo4k = (0x1000 - (engine->sgdma->firstDescLo & 0xFFF)) / sizeof(DMA_DESCRIPTOR) - 1;

    // set the number of adjacent descriptors for the first fetch
    ULONG firstAdj = adjTotal < adjMax ? adjTotal : adjMax;
    engine->sgdma->firstDescAdj = adjTo4k < firstAdj ? adjTo4k : firstAdj;

    // set the number of adjacent descriptors for subsequent fetches
    ULONG nextAdjMax = adjMax - 1;
    for (UINT i = 0; i < numDesc; i++) {
        // if not last desc then get total desc adj to next desc, else last desc has no next desc
        const ULONG nextAdjTotal = (i != adjTotal) ? adjTotal - (i + 1) : 0;
        const ULONG nextAdjTo4k = (0x1000 - (desc[i].nextLo & 0xFFF)) / sizeof(DMA_DESCRIPTOR) - 1;

        ULONG nextAdj = nextAdjTotal < nextAdjMax ? nextAdjTotal : nextAdjMax;
        if (nextAdj > nextAdjTo4k) {
            nextAdj = nextAdjTo4k;
        }

        desc[i].control |= (nextAdj << 8);

        // update current max adj count for this block
        if (nextAdjMax != 0) {
            nextAdjMax--;
        } else { // wrap-around
            nextAdjMax = adjMax;
        }
    }
}

static BOOLEAN EngineExists(PXDMA_DEVICE xdma, DirToDev dir, ULONG channel) {
    PUCHAR configBarAddr = (PUCHAR)xdma->bar[xdma->configBarIdx];
    const ULONG offset = (dir * BLOCK_OFFSET) + (channel * ENGINE_OFFSET);
    XDMA_ENGINE_REGS* engineRegs = (XDMA_ENGINE_REGS*)(configBarAddr + offset);
    UINT32 engineID = engineRegs->identifier;
    return (engineID & XDMA_ID_MASK) == XDMA_ID;
}

static NTSTATUS EngineCreate(PXDMA_DEVICE xdma, XDMA_ENGINE* engine, DirToDev dir, ULONG channel,
                             ULONG engineIndex, ULONG engineId) {

    NTSTATUS status;

    engine->parentDevice = xdma;
    engine->channel = channel;
    engine->dir = dir;
    const ULONG offset = (dir * BLOCK_OFFSET) + (channel * ENGINE_OFFSET);
    PUCHAR configBarAddr = (PUCHAR)xdma->bar[xdma->configBarIdx];
    engine->regs = (XDMA_ENGINE_REGS*)(configBarAddr + offset);
    engine->sgdma = (XDMA_SGDMA_REGS*)(configBarAddr + offset + SGDMA_BLOCK_OFFSET);

    // This simplified driver supports only Memory-Mapped (AXI-MM) engines.
    engine->type = EngineType_MM;
    ASSERTMSG("Streaming engines are not supported by this simplified driver",
              (engine->regs->identifier & XDMA_ID_ST_BIT) == 0);

    // Incremental or Non-Incremental address mode? 0 = inc, 1=non-inc
    engine->addressMode = (engine->regs->control & XDMA_CTRL_NON_INCR_ADDR) != 0;

    // set interrupt sources
    EngineConfigureInterrupt(engine, engineIndex, engineId);

    // capture alignment requirements
    EngineGetAlignments(engine);

    // create and bind dma descriptor buffer to hw
    status = EngineCreateDescriptorBuffer(engine);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "EngineCreateDescriptorBuffer() failed: %!STATUS!",
                   status);
        return status;
    }

    // allocate wdf dma transaction object
    status = WdfDmaTransactionCreate(xdma->dmaEnabler, WDF_NO_OBJECT_ATTRIBUTES,
                                     &engine->dmaTransaction);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "WdfDmaTransactionCreate() failed: %!STATUS!", status);
        return status;
    }

    engine->work = EngineProcessTransfer;

    // Initialize engine request tracking and lock
    engine->isReqPending = FALSE;

    status = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &engine->engineLock);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "WdfSpinLockCreate failed: %!STATUS!", status);
        return status;
    }

    // Initialize the watchdog timer and its associated DPC for DMA timeout handling
    KeInitializeTimerEx(&engine->watchdogTimer, NotificationTimer);
    KeInitializeDpc(&engine->watchdogDpc, EngineWatchdogDpc, engine);

    engine->enabled = TRUE;

    return status;
}

static void EngineGetAlignments(IN OUT XDMA_ENGINE *engine) {

    UINT32 alignments = engine->regs->alignments;
    UINT32 align_bytes = (alignments & 0x00ff0000U) >> 16;
    UINT32 granularity_bytes = (alignments & 0x0000ff00U) >> 8;
    UINT32 address_bits = (alignments & 0x000000ffU);

    if (alignments) {
        engine->alignAddr = align_bytes;
        engine->alignLength = granularity_bytes;
        engine->alignAddrBits = address_bits;
    } else { // Some default values if alignments are unspecified
        engine->alignAddr = 1;
        engine->alignLength = 1;
        engine->alignAddrBits = 64;
    }

    TraceVerbose(DBG_INIT, "engine[%u][%u] alignments: bytes=%u, granularity=%u, addrBits=%u",
                 engine->channel, engine->dir, engine->alignAddr, engine->alignLength, engine->alignAddrBits);

}

static UINT32 EngineStatus(IN XDMA_ENGINE *engine, IN BOOLEAN clear)
// read and optionally clear the engine status
{
    UINT32 status = 0;

    if (clear) { // request to clear engine status after read? 
        TraceVerbose(DBG_DMA, "%s_%u engine status cleared after next read",
                     DirectionToString(engine->dir), engine->channel);
        status = engine->regs->statusRC;
    } else {
        status = engine->regs->status;
    }

    TraceInfo(DBG_DMA, "%s_%u status=0x%08x (%s%s%s%s%s%s%s%s%s)",
              DirectionToString(engine->dir), engine->channel, status,
              (status & XDMA_BUSY_BIT) ? "BUSY " : "IDLE ",
              (status & XDMA_DESCRIPTOR_STOPPED_BIT) ? "DESCRIPTOR_STOPPED " : " ",
              (status & XDMA_DESCRIPTOR_COMPLETED_BIT) ? "DESCRIPTOR_COMPLETED " : " ",
              (status & XDMA_ALIGN_MISMATCH_BIT) ? "ALIGN_MISMATCH " : " ",
              (status & XDMA_MAGIC_STOPPED_BIT) ? "MAGIC_STOPPED " : " ",
              (status & XDMA_FETCH_STOPPED_BIT) ? "FETCH_STOPPED" : " ",
              (status & XDMA_STAT_READ_ERROR) ? "READ_ERROR " : " ",
              (status & XDMA_STAT_DESCRIPTOR_ERROR) ? "DESCRIPTOR_ERROR " : " ",
              (status & XDMA_IDLE_STOPPED_BIT) ? "IDLE_STOPPED" : " ");

    return status;
}

BOOLEAN XDMA_EngineProgramDma(IN WDFDMATRANSACTION Transaction, IN WDFDEVICE Device,
                              IN WDFCONTEXT context, IN WDF_DMA_DIRECTION Direction,
                              IN PSCATTER_GATHER_LIST SgList)
    // this programs the engine to start a dma transfer
{
    UNREFERENCED_PARAMETER(Device);

    WDFREQUEST request = WdfDmaTransactionGetRequest(Transaction);
    WDF_REQUEST_PARAMETERS params;
    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(request, &params);
    LONGLONG deviceOffset = (Direction == WdfDmaDirectionWriteToDevice) ?
        (SIZE_T)params.Parameters.Write.DeviceOffset :
        (SIZE_T)params.Parameters.Read.DeviceOffset;
    size_t numBytesTransferred;

    // get virtual and physical pointers to descriptor buffer
    XDMA_ENGINE * engine = (XDMA_ENGINE*)context;
    DMA_DESCRIPTOR *descriptor = (DMA_DESCRIPTOR*)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
    PHYSICAL_ADDRESS descBufferLA = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);

    WdfSpinLockAcquire(engine->engineLock);
    if (TRUE == engine->isReqPending) {
        WdfSpinLockRelease(engine->engineLock);
        ASSERTMSG("Request is already pending", (1 == 1));
    }
    else {
        WdfSpinLockRelease(engine->engineLock);
    }

    //validate num descs
    if (SgList->NumberOfElements > engine->capacity) {
        TraceError(DBG_INIT, "Too many descriptors...");
        goto ErrExit;
    }

    // offset into the transaction (if it is split)
    numBytesTransferred = WdfDmaTransactionGetBytesTransferred(Transaction);
    deviceOffset += numBytesTransferred;

    TraceVerbose(DBG_DMA, "device addr=%lld, num descriptors=%d",
                 deviceOffset, SgList->NumberOfElements);

    for (ULONG i = 0; i < SgList->NumberOfElements; i++) {
        descriptor[i].control = XDMA_DESC_MAGIC;
        descriptor[i].numBytes = SgList->Elements[i].Length;
        ULONG hostAddrLo = SgList->Elements[i].Address.LowPart;
        LONG hostAddrHi = SgList->Elements[i].Address.HighPart;
        if (Direction == WdfDmaDirectionWriteToDevice) {
            // source is host memory
            descriptor[i].srcAddrLo = hostAddrLo;
            descriptor[i].srcAddrHi = hostAddrHi;
            descriptor[i].dstAddrLo = LIMIT_TO_32(deviceOffset);
            descriptor[i].dstAddrHi = LIMIT_TO_32(deviceOffset >> 32);
        } else {
            // destination is host memory
            descriptor[i].srcAddrLo = LIMIT_TO_32(deviceOffset);
            descriptor[i].srcAddrHi = LIMIT_TO_32(deviceOffset >> 32);
            descriptor[i].dstAddrLo = hostAddrLo;
            descriptor[i].dstAddrHi = hostAddrHi;
        }

        // next descriptor bus address 
        descBufferLA.QuadPart += sizeof(DMA_DESCRIPTOR);

        // non-last descriptor(s)? 
        if ((i + 1) < SgList->NumberOfElements) {
            descriptor[i].nextLo = descBufferLA.LowPart;
            descriptor[i].nextHi = descBufferLA.HighPart;
        } else { // last descriptor
            descriptor[i].nextLo = 0;
            descriptor[i].nextHi = 0;
            // stop engine and request an interrupt from the engine
            descriptor[i].control |= (XDMA_DESC_STOP_BIT | XDMA_DESC_COMPLETED_BIT);
        }
        if (engine->addressMode == AddressMode_Contiguous) { // incremental address mode
            deviceOffset += SgList->Elements[i].Length;
        }

        if (FALSE == DescriptorIsAligned(engine, &(descriptor[i]))) {
            TraceWarning(DBG_DMA, "Error: Dma Transfer is not aligned");
        }
    }

    OptimizeDescriptors(engine, descriptor, SgList->NumberOfElements);

    for (ULONG i = 0; i < SgList->NumberOfElements; i++) {
        DumpDescriptor(&(descriptor[i]));
    }

    MemoryBarrier();
    markReqPending(engine);
    // start the engine
    EngineStart(engine);
    MemoryBarrier();

    // Arm the watchdog timer. If the DMA does not complete within
    // XDMA_DMA_TIMEOUT_SECONDS, EngineWatchdogDpc will fire and complete
    // the request with STATUS_TIMEOUT.
    LARGE_INTEGER watchdogTimeout;
    watchdogTimeout.QuadPart = XDMA_DMA_TIMEOUT_100NS;
    KeSetTimer(&engine->watchdogTimer, watchdogTimeout, &engine->watchdogDpc);

    return TRUE;

ErrExit:
    // FIXME: Current framework ignores the FALSE return value.
    // Therefore cleaning the request here itself.
    {
        NTSTATUS status = STATUS_UNSUCCESSFUL;
        WdfDmaTransactionRelease(engine->dmaTransaction);
        WdfRequestComplete(request, status);
    }
    return FALSE;
}

void CountChannels(IN PXDMA_DEVICE xdma, OUT ULONG *h2cCount,OUT ULONG *c2hCount)
{
    *h2cCount = *c2hCount = 0;

    // H2C Channels
    for (ULONG ch = 0; ch < XDMA_MAX_NUM_CHANNELS; ch++) {
        if (EngineExists(xdma, H2C, ch)) {
            (*h2cCount)++;
        }
    }

    // C2H Channels
    for (ULONG ch = 0; ch < XDMA_MAX_NUM_CHANNELS; ch++) {
        if (EngineExists(xdma, C2H, ch)) {
            (*c2hCount)++;
        }
    }
}

NTSTATUS ProbeEngines(IN PXDMA_DEVICE xdma) {
    PAGED_CODE();

    ULONG engineIndex = 0;

    // iterate over H2C (FPGA performs PCIe reads towards FPGA),
    // then C2H (FPGA performs PCIe writes from FPGA)
    ULONG h2cChannelMax, c2hChannelMax;
    CountChannels(xdma, &h2cChannelMax, &c2hChannelMax);

    UINT dir = H2C;
    for (ULONG ch = 0; ch < xdma->h2cChannelMax; ch++) {
        if (EngineExists(xdma, dir, ch)) {
            XDMA_ENGINE* engine = &(xdma->engines[ch][dir]);
            NTSTATUS status = EngineCreate(xdma, engine, dir, ch, engineIndex, engineIndex);
            if (!NT_SUCCESS(status)) {
                TraceError(DBG_INIT, "EngineCreate failed! %!STATUS!", status);
                return status;
            }
            engineIndex++;
            TraceInfo(DBG_INIT, "%s_%u engine created (AXI-MM)",
                DirectionToString(dir), ch);
        }
        else {     // skip inactive engines
            TraceInfo(DBG_INIT, "Skipping non-existing engine %s_%u",
                DirectionToString(dir), ch);
        }
    }

    dir = C2H;
    for (ULONG ch = 0; ch < xdma->c2hChannelMax; ch++) {
        if (EngineExists(xdma, dir, ch)) {
            XDMA_ENGINE* engine = &(xdma->engines[ch][dir]);
            NTSTATUS status = EngineCreate(xdma, engine, dir, ch, engineIndex, h2cChannelMax + ch);
            if (!NT_SUCCESS(status)) {
                TraceError(DBG_INIT, "EngineCreate failed! %!STATUS!", status);
                return status;
            }
            engineIndex++;
            TraceInfo(DBG_INIT, "%s_%u engine created (AXI-MM)",
                DirectionToString(dir), ch);
        }
        else {     // skip inactive engines
            TraceInfo(DBG_INIT, "Skipping non-existing engine %s_%u",
                DirectionToString(dir), ch);
        }
    }

    return STATUS_SUCCESS;
}

void EngineStart(IN XDMA_ENGINE *engine) {
    engine->regs->controlW1S = XDMA_CTRL_RUN_BIT;
    TraceInfo(DBG_DMA, "%s_%u engine started (control=0x%08x)",
              DirectionToString(engine->dir), engine->channel, engine->regs->control);
}

void EngineStop(IN XDMA_ENGINE *engine) {
    engine->regs->controlW1C = XDMA_CTRL_RUN_BIT;
    TraceInfo(DBG_DMA, "%s_%u engine stopped (control=0x%08x)",
              DirectionToString(engine->dir), engine->channel, engine->regs->control);
}

void EngineEnableInterrupt(IN XDMA_ENGINE* engine) {
    if (!engine) {
        TraceError(DBG_IRQ, "engine ptr is NULL!");
        return;

    }
    engine->parentDevice->interruptRegs->channelIntEnableW1S = engine->irqBitMask;
    TraceInfo(DBG_IRQ, "%s_%u enabled interrupt", DirectionToString(engine->dir), engine->channel);
}

void EngineDisableInterrupt(IN XDMA_ENGINE* engine) {
    if (!engine) {
        TraceError(DBG_IRQ, "engine ptr is NULL");
        return;
    }
    engine->parentDevice->interruptRegs->channelIntEnableW1C = engine->irqBitMask;
    TraceInfo(DBG_IRQ, "%s_%u disabled interrupt", DirectionToString(engine->dir), engine->channel);
}

char* DirectionToString(const DirToDev dir) {
    return dir == H2C ? "H2C" : "C2H";
}
