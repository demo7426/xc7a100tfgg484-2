/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	dma_engine.c
作  者:	钱锐      版本: V1.0     新建日期: 2026.09.06
描  述: dma 引擎
备  注:
修改记录:

  1.  日期: 2026.09.06
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include "dma_engine.h"
#include "trace.h"
#include "device.h"
#include "xdma_public.h"
#include "reg.h"

#ifdef DBG
#include "dma_engine.tmh"
#endif

//检查当前通道的DMA是否存在
static BOOLEAN EngineExists(_In_ DEVICE_CONTEXT* device_context, DirToDev dir, ULONG channel)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");
    PUCHAR bar_addr = device_context->bar_infos[CONFIG_BAR_INDEX].kernel_virtual_address;
    ULONG offset = (dir * BLOCK_OFFSET) + (channel * ENGINE_OFFSET);

    XDMA_ENGINE_REGS* regs = (XDMA_ENGINE_REGS*)(bar_addr + offset);

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");

    return (regs->identifier & XDMA_ID_MASK) == XDMA_ID;
}

//计算DMA的有效通道数量
static VOID CountChannels(_In_ DEVICE_CONTEXT* device_context, _Out_ ULONG* h2c_count, _Out_ ULONG* c2h_count)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    if (!device_context || !h2c_count || !c2h_count)
    {
        TraceError(DBG_INIT, "%!FUNC! input parameter is error, device_context = %p, h2c_count = %p, c2h_count = %p", device_context, h2c_count, c2h_count);
        return;
    }

    *h2c_count = 0;
    *c2h_count = 0;

    for (ULONG ch = 0; ch < XDMA_MAX_NUM_CHANNELS; ++ch)
    {
        EngineExists(device_context, H2C, ch) == TRUE ? ++h2c_count : 0;
        EngineExists(device_context, C2H, ch) == TRUE ? ++c2h_count : 0;
    }

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

static UINT32 EnginReadStatus(_In_ PDMA_ENGINE engine, BOOLEAN clear)
{
    return clear ? engine->regs->statusRC : engine->regs->status;
}



NTSTATUS ProbeEngine(_In_ DEVICE_CONTEXT* device_contex)
{
    UNREFERENCED_PARAMETER(device_contex);

    TraceVerbose(DBG_INIT, "%!FUNC! is start.");
    NTSTATUS status = STATUS_SUCCESS;

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
    return status;
}

NTSTATUS CloseEngine(_In_ DEVICE_CONTEXT* device_contex)
{
    UNREFERENCED_PARAMETER(device_contex);

    TraceVerbose(DBG_INIT, "%!FUNC! is start.");
    NTSTATUS status = STATUS_SUCCESS;

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
    return status;
}

VOID EngineStart(_In_ PDMA_ENGINE engine)
{
    UNREFERENCED_PARAMETER(engine);

    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

VOID EngineStop(_In_ PDMA_ENGINE engine)
{
    UNREFERENCED_PARAMETER(engine);

    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

VOID EngineProcessChannelInterrupt(_In_ DEVICE_CONTEXT* device_contex)
{
    UNREFERENCED_PARAMETER(device_contex);

    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

BOOLEAN EvtProgramDma(_In_ WDFDMATRANSACTION transaction, _In_ WDFDEVICE device, _In_ WDFCONTEXT wdf_context, _In_ WDF_DMA_DIRECTION direction, _In_ PSCATTER_GATHER_LIST sg_list)
{
    UNREFERENCED_PARAMETER(transaction);
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(wdf_context);
    UNREFERENCED_PARAMETER(direction);
    UNREFERENCED_PARAMETER(sg_list);

    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
    return TRUE;
}
