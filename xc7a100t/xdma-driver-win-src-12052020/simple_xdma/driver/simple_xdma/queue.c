/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	queue.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.09.01
描  述: Irp请求队列文件
备  注:
修改记录:

  1.  日期: 2026.09.01
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include "queue.h"
#include "dma_engine.h"
#include "trace.h"

#ifdef DBG
#include "queue.tmh"
#endif

DMA_ENGINE* GetEngineFromQueue(_In_ WDFQUEUE Queue)
{
    PQUEUE_CONTEXT ctx = GetQueueContext(Queue);
    return ctx->engine;
}

VOID EVT_WDF_IO_Queue_IO_Device_Control(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_ size_t OutputBufferLength, _In_ size_t InputBufferLength, _In_ ULONG IoControlCode)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Request);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(IoControlCode);


    return;
}

VOID EVT_WDF_IO_Queue_IO_Read(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_  size_t Length)
{
    UNREFERENCED_PARAMETER(Length);

    TraceVerbose(DBG_INIT, "%!FUNC! is enter.");

    PDMA_ENGINE engine = GetEngineFromQueue(Queue);
    if (!engine || !engine->enabled)
    {
        WdfRequestComplete(Request, STATUS_DEVICE_NOT_READY);
        return;
    }

    NTSTATUS status = STATUS_SUCCESS;

    status = WdfDmaTransactionInitializeUsingRequest(engine->dmaTransaction, Request, EvtProgramDma, WdfDmaDirectionReadFromDevice);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_IO, "%!FUNC!: WdfIoQueueCreate failed: %!STATUS!", status);

        WdfRequestComplete(Request, status);
        return;
    }

    status = WdfDmaTransactionExecute(engine->dmaTransaction, engine);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_IO, "%!FUNC!: WdfDmaTransactionExecute failed: %!STATUS!", status);

        WdfRequestComplete(Request, status);
        return;
    }

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

VOID EVT_WDF_IO_Queue_IO_Write(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_  size_t Length)
{
    UNREFERENCED_PARAMETER(Length);

    TraceVerbose(DBG_INIT, "%!FUNC! is enter.");

    PDMA_ENGINE engine = GetEngineFromQueue(Queue);
    if (!engine || !engine->enabled)
    {
        WdfRequestComplete(Request, STATUS_DEVICE_NOT_READY);
        return;
    }

    NTSTATUS status = STATUS_SUCCESS;

    status = WdfDmaTransactionInitializeUsingRequest(engine->dmaTransaction, Request, EvtProgramDma, WdfDmaDirectionWriteToDevice);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_IO, "%!FUNC!: WdfIoQueueCreate failed: %!STATUS!", status);

        WdfRequestComplete(Request, status);
        return;
    }

    status = WdfDmaTransactionExecute(engine->dmaTransaction, engine);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_IO, "%!FUNC!: WdfDmaTransactionExecute failed: %!STATUS!", status);

        WdfRequestComplete(Request, status);
        return;
    }

    // 请求完成在 EvtProgramDma 或 EngineProcessTransfer 中回调

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

VOID EVT_WDF_IO_Queue_IO_Stop(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_ ULONG ActionFlags)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Request);
    UNREFERENCED_PARAMETER(ActionFlags);
}
