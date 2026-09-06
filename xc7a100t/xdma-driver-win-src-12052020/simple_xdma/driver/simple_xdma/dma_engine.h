/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	dma_engine.h
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

#ifndef __DMA_ENGINE_H__
#define __DMA_ENGINE_H__

#include <ntddk.h>
#include <wdf.h>

#define XDMA_MAX_NUM_CHANNELS   (4)
#define XDMA_NUM_DIRECTIONS     (2)

typedef struct _DEVICE_CONTEXT DEVICE_CONTEXT;
typedef struct _XDMA_ENGINE_REGS XDMA_ENGINE_REGS;
typedef struct _XDMA_SGDMA_REGS XDMA_SGDMA_REGS;

/// Direction of the DMA transfer/engine
typedef enum DirToDev_t {
    H2C = 0, // Host-to-Card - write to device
    C2H = 1  // Card-to-Host - read from device
} DirToDev;

//DMA 引擎抽象
typedef struct _DMA_ENGINE
{
    DEVICE_CONTEXT* parent_device;

    //寄存器访问
    volatile XDMA_ENGINE_REGS* regs;
    volatile XDMA_SGDMA_REGS* sgdma;

}DMA_ENGINE, *PDMA_ENGINE;

//探测并初始化所有 DMA 引擎
NTSTATUS ProbeEngine(_In_ DEVICE_CONTEXT* device_contex);

//关闭所有引擎
NTSTATUS CloseEngine(_In_ DEVICE_CONTEXT* device_contex);

//启动引擎
VOID EngineStart(_In_ PDMA_ENGINE engine);

//停止引擎
VOID EngineStop(_In_ PDMA_ENGINE engine);

//引擎中断处理（DPC中调用）
VOID EngineProcessChannelInterrupt(_In_ DEVICE_CONTEXT* device_contex);

//WDF ProgramDMA回调 - 编程描述符并启动传输
BOOLEAN EvtProgramDma(_In_ WDFDMATRANSACTION transaction, _In_ WDFDEVICE device, _In_ WDFCONTEXT wdf_context, _In_ WDF_DMA_DIRECTION direction, _In_ PSCATTER_GATHER_LIST sg_list);

#endif // !__DMA_ENGINE_H__
