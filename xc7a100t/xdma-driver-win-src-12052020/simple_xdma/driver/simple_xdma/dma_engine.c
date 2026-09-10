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
#include "interrupt.h"

#ifdef DBG
#include "dma_engine.tmh"
#endif

static VOID EngineProcessTransfer(_In_ PDMA_ENGINE engine);

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
        (EngineExists(device_context, H2C, ch) == TRUE) ? (*h2c_count)++ : 0;
        (EngineExists(device_context, C2H, ch) == TRUE) ? (*c2h_count)++ : 0;
    }

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

//引擎读取状态
//函数功能：读取DMA引擎的状态寄存器，可选是否读取【带清除功能】的状态位
static UINT32 EnginReadStatus(_In_ PDMA_ENGINE engine, BOOLEAN clear)
{
    // C语言三元运算符：判断clear标志
    // clear == TRUE  → 读取 statusRC 寄存器（Read‑Clear，读即清零）
    // clear == FALSE → 读取普通 status 寄存器（只读，读完状态保持不变）
    return clear ? engine->regs->statusRC : engine->regs->status;
}

//引擎创建描述符缓冲区
static NTSTATUS EnginCreateDescriptorBuffer(_Inout_ PDMA_ENGINE engine)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");
    NTSTATUS status = STATUS_SUCCESS;

    // ===================== 计算描述符缓冲区总字节大小 =====================
    // XDMA_MAX_TRANSFER_SIZE：单次DMA最大传输字节数
    // PAGE_SIZE：内存页大小，用来估算最多需要多少个SG描述符
    // +2：预留冗余描述符，防止边界对齐溢出
    // sizeof(DMA_DESCRIPTOR)：单个XDMA SG描述符结构体字节大小
    SIZE_T buffer_size = (XDMA_MAX_TRANSFER_SIZE / PAGE_SIZE + 2) * sizeof(DMA_DESCRIPTOR);     

    PHYSICAL_ADDRESS desc_pa;       //物理地址
    PVOID desc_va = NULL;           //虚拟地址

    // ===================== 创建WDF公共DMA缓冲区 =====================
    // WdfCommonBufferCreate：分配**物理连续**的内核共享内存
    // engine->parentDevice->dmaEnabler：设备的DMA启用对象（驱动初始化时创建）
    // buffer_size：申请内存字节数
    // WDF_NO_OBJECT_ATTRIBUTES：无自定义对象属性
    // &engine->descBuffer：输出WDFCOMMONBUFFER句柄，保存到DMA引擎结构体中
    status = WdfCommonBufferCreate(engine->parentDevice->dmaEnabler, buffer_size, WDF_NO_OBJECT_ATTRIBUTES, &engine->descBuffer);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "%!FUNC!: WdfCommonBufferCreate failed: %!STATUS!", status);
        return status;
    }

    // 计算该缓冲区最多能存放多少个DMA描述符，存入引擎容量字段
    engine->capacity = (UINT32)(buffer_size / sizeof(DMA_DESCRIPTOR));

    // 获取缓冲区对应的**DMA逻辑地址（物理地址）**，FPGA硬件寄存器填写这个地址
    desc_pa = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);     
    
    // 获取**内核虚拟地址**，驱动代码通过这个指针读写描述符内存（CPU访问）
    desc_va = WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);               

    RtlZeroMemory(desc_va, buffer_size);

    engine->sgdma->firstDescLo = desc_pa.LowPart;
    engine->sgdma->firstDescHi = desc_pa.HighPart;
    engine->sgdma->firstDescAdj = 0;                    // 描述符起始地址偏移调整值，这里不做偏移，填0

    TraceInfo(
        DBG_INIT, 
        "%!FUNC!: %s_%u desc buffer pa = 0x%08x%08x, capacity = %u", 
        (engine->dir == H2C? "H2c": "C2H"), 
        engine->channel, 
        desc_pa.HighPart, 
        desc_pa.LowPart,
        engine->capacity
    );

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
    return status;
}

//配置DMA引擎中断
static VOID EngineConfigureInterrupt(_Inout_ PDMA_ENGINE engine, ULONG engine_id)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");
    
    // 计算当前引擎对应的中断掩码：1左移engine_id位，单独标识该通道的中断位
    engine->irqBitMask = 1 << engine_id;

    // XDMA_CTRL_IE_ALL：宏定义，代表**开启该引擎全部中断使能位**（传输完成、错误、超时等）
    UINT32 ie = XDMA_CTRL_IE_ALL;

    // intEnableMaskW1S：W1S寄存器（Write‑1‑to‑Set，写1置位）
    // 向该寄存器写入ie，会把对应中断允许位置1，打开硬件中断开关
    engine->regs->intEnableMaskW1S = ie;            //**选择哪些事件能够产生中断**（中断源过滤）

    // controlW1S：控制寄存器同样是W1S类型，写入ie，使能DMA引擎本身的中断上报能力
    engine->regs->controlW1S = ie;                  //**打开整个 DMA 通道的中断输出开关**（通道闸门）

    TraceInfo(
        DBG_INIT, 
        "%!FUNC!: %s_%u irqBitMask=0x%08x",
        (engine->dir == H2C ? "H2C" : "C2H"), 
        engine->channel,
        engine->irqBitMask
    );

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

//引擎创建
static NTSTATUS EngineCreate(_In_ DEVICE_CONTEXT* device_context, _Inout_ PDMA_ENGINE engine, DirToDev dir, ULONG channel, ULONG engine_id)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    engine->parentDevice = device_context;
    engine->channel = channel;
    engine->dir = dir;

    NTSTATUS status = STATUS_SUCCESS;
    PUCHAR bar_addr = device_context->bar_infos[CONFIG_BAR_INDEX].kernel_virtual_address;
    ULONG offset = (dir * BLOCK_OFFSET) + (channel * ENGINE_OFFSET);

    engine->regs = (XDMA_ENGINE_REGS*)(bar_addr + offset);
    engine->sgdma = (XDMA_SGDMA_REGS*)(bar_addr + offset + SGDMA_BLOCK_OFFSET);

    EngineConfigureInterrupt(engine, engine_id);

    status = EnginCreateDescriptorBuffer(engine);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "%!FUNC!: EnginCreateDescriptorBuffer failed: %!STATUS!", status);
        return status;
    }

    status = WdfDmaTransactionCreate(device_context->dmaEnabler, WDF_NO_OBJECT_ATTRIBUTES, &engine->dmaTransaction);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "%!FUNC!: WdfDmaTransactionCreate failed: %!STATUS!", status);
        return status;
    }

    status = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &engine->engineLock);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "%!FUNC!: WdfSpinLockCreate failed: %!STATUS!", status);
        return status;
    }

    KeInitializeEvent(&engine->completionEvent, NotificationEvent, FALSE);

    WdfSpinLockAcquire(engine->engineLock);
    engine->isReqPending = FALSE;
    WdfSpinLockRelease(engine->engineLock);

    engine->enabled = TRUE;

    TraceInfo(
        DBG_INIT, 
        "%!FUNC!: %s_%u engine created",
        (dir == H2C ? "H2C" : "C2H"), 
        channel
    );

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");

    return status;
}

NTSTATUS ProbeEngine(_In_ DEVICE_CONTEXT* device_contex)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");
    NTSTATUS status = STATUS_SUCCESS;

    ULONG h2c_count = 0;
    ULONG c2h_count = 0;

    ULONG ch = 0;

    CountChannels(device_contex, &h2c_count, &c2h_count);
    
    device_contex->h2c_count = h2c_count;
    device_contex->c2h_count = c2h_count;

    ULONG engineId = 0;
    for (ch = 0; ch < h2c_count; ch++)
    {
        if (EngineExists(device_contex, H2C, ch))
        {
            status = EngineCreate(device_contex, &(device_contex->engines[ch][H2C]), H2C, ch, engineId++);
            if (!NT_SUCCESS(status))
            {
                TraceError(DBG_INIT, "%!FUNC!: EngineCreate failed: %!STATUS!", status);
                return status;
            }
        }
    }

    for (ch = 0; ch < c2h_count; ch++)
    {
        if (EngineExists(device_contex, C2H, ch))
        {
            status = EngineCreate(device_contex, &(device_contex->engines[ch][C2H]), C2H, ch, engineId++);
            if (!NT_SUCCESS(status))
            {
                TraceError(DBG_INIT, "%!FUNC!: EngineCreate failed: %!STATUS!", status);
                return status;
            }
        }
    }

    // ProbeEngine 末尾，所有引擎创建完毕后：
    UINT32 channel_mask = 0;
    for (ch = 0; ch < device_contex->h2c_count; ch++) {
        channel_mask |= device_contex->engines[ch][H2C].irqBitMask;
    }
    for ( ch = 0; ch < device_contex->c2h_count; ch++) {
        channel_mask |= device_contex->engines[ch][C2H].irqBitMask;
    }
    device_contex->interrupt_regs->channelIntEnableW1S = channel_mask;

    TraceVerbose(DBG_INIT, "%!FUNC!: h2c_count = %u, c2h_count = %u.", h2c_count, c2h_count);

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");

    return status;
}

NTSTATUS CloseEngine(_In_ DEVICE_CONTEXT* device_contex)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");
    NTSTATUS status = STATUS_SUCCESS;
    
    for (ULONG ch = 0; ch < device_contex->h2c_count; ch++)
    {
        device_contex->engines[ch][H2C].enabled = FALSE;
    }
    
    for (ULONG ch = 0; ch < device_contex->c2h_count; ch++)
    {
        device_contex->engines[ch][C2H].enabled = FALSE;
    }

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");

    return status;
}

VOID EngineStart(_In_ PDMA_ENGINE engine)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    engine->regs->controlW1S = XDMA_CTRL_RUN_BIT;

    TraceInfo(
        DBG_DMA, 
        "%!FUNC!: %s_%u started",
        (engine->dir == H2C ? "H2C" : "C2H"), 
        engine->channel
    );

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

VOID EngineStop(_In_ PDMA_ENGINE engine)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    engine->regs->controlW1C = XDMA_CTRL_RUN_BIT;
    
    TraceInfo(
        DBG_DMA,
        "%!FUNC!: %s_%u stopped",
        (engine->dir == H2C ? "H2C" : "C2H"),
        engine->channel
    );

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

VOID EngineProcessChannelInterrupt(_In_ DEVICE_CONTEXT* device_contex)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    UINT32 chan_irq = device_contex->interrupt_regs->channelIntRequest;
    if (chan_irq == 0)
    {
        TraceInfo(DBG_IRQ, "%!FUNC!: chan_irq = %u", chan_irq);
        return;
    }

    //遍历所有已启动的引擎，匹配中断位
    for (ULONG ch = 0; ch < device_contex->h2c_count; ch++)
    {
        PDMA_ENGINE engine = &device_contex->engines[ch][H2C];
        if (engine && engine->enabled && (chan_irq & engine->irqBitMask))
        {
            EngineProcessTransfer(engine);
        }
    }
    
    for (ULONG ch = 0; ch < device_contex->c2h_count; ch++)
    {
        PDMA_ENGINE engine = &device_contex->engines[ch][C2H];
        if (engine && engine->enabled && (chan_irq & engine->irqBitMask))
        {
            EngineProcessTransfer(engine);
        }
    }

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

BOOLEAN EvtProgramDma(_In_ WDFDMATRANSACTION transaction, _In_ WDFDEVICE device, _In_ WDFCONTEXT wdf_context, _In_ WDF_DMA_DIRECTION direction, _In_ PSCATTER_GATHER_LIST sg_list)
{
    UNREFERENCED_PARAMETER(device);

    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    PDMA_ENGINE engine = (PDMA_ENGINE)wdf_context;

    PHYSICAL_ADDRESS desc_pa;       //物理地址
    DMA_DESCRIPTOR* desc = NULL;           //虚拟地址

    desc = (DMA_DESCRIPTOR*)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
    desc_pa = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);

    if (sg_list->NumberOfElements > engine->capacity)
    {
        TraceError(DBG_DMA, "%!FUNC!: too many sg elelments: %u > %u", sg_list->NumberOfElements, engine->capacity);
        return FALSE;
    }

    //获取 WriteFile/ReadFile 传入的设备偏移
    WDFREQUEST request = WdfDmaTransactionGetRequest(transaction);
    if (!request)
    {
        TraceError(DBG_DMA, "%!FUNC!: WdfDmaTransactionGetRequest failed");
        return FALSE;
    }

    WDF_REQUEST_PARAMETERS params;
    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(request, &params);

    LONGLONG device_offset = 0;

    device_offset = (direction == WdfDmaDirectionWriteToDevice) ? (LONGLONG)params.Parameters.Write.DeviceOffset : (LONGLONG)params.Parameters.Read.DeviceOffset;

    size_t bytes_done = WdfDmaTransactionGetBytesTransferred(transaction);
    device_offset += bytes_done;

    //填充描述符链表
    for (ULONG i = 0; i < sg_list->NumberOfElements; ++i)
    {
        desc[i].control = XDMA_DESC_MAGIC;
        desc[i].numBytes = sg_list->Elements[i].Length;

        if (direction == WdfDmaDirectionWriteToDevice)
        {
            // H2C: src=主机内存, dst=设备地址
            desc[i].srcAddrLo = sg_list->Elements[i].Address.LowPart;
            desc[i].srcAddrHi = sg_list->Elements[i].Address.HighPart;
            desc[i].dstAddrLo = (UINT32)(device_offset & 0xffffffff);
            desc[i].dstAddrHi = (UINT32)(device_offset >> 32);
        }
        else if (direction == WdfDmaDirectionReadFromDevice)
        {
            // C2H: src=设备地址, dst=主机内存
            desc[i].srcAddrLo = (UINT32)(device_offset & 0xffffffff);
            desc[i].srcAddrHi = (UINT32)(device_offset >> 32);
            desc[i].dstAddrLo = sg_list->Elements[i].Address.LowPart;
            desc[i].dstAddrHi = sg_list->Elements[i].Address.HighPart;
        }
        else {
        }

        desc_pa.QuadPart += sizeof(DMA_DESCRIPTOR);

        if ((i + 1) < sg_list->NumberOfElements)
        {
            desc[i].nextLo = desc_pa.LowPart;
            desc[i].nextHi = desc_pa.HighPart;
        }
        else
        {
            // 最后一项：停止引擎 + 请求中断
            desc[i].nextLo = 0;
            desc[i].nextHi = 0;
            desc[i].control |= (XDMA_DESC_STOP_BIT | XDMA_DESC_COMPLETED_BIT);
        }

        device_offset += sg_list->Elements[i].Length;
    }

    //标记请求中，启动引擎
    WdfSpinLockAcquire(engine->engineLock);
    engine->isReqPending = TRUE;
    WdfSpinLockRelease(engine->engineLock);

    EngineStart(engine);

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");

    return TRUE;
}

static VOID EngineProcessTransfer(_In_ PDMA_ENGINE engine)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is start.");

    NTSTATUS status = STATUS_SUCCESS;
    WDFREQUEST request;
    UINT32 engine_status = 0;
    DMA_DESCRIPTOR* desc = NULL;

    WdfSpinLockAcquire(engine->engineLock);

    if (engine->isReqPending == FALSE)
    {
        WdfSpinLockRelease(engine->engineLock);

        TraceWarning(DBG_DMA, "%!FUNC!: %s_%u spurious interrupt",
            (engine->dir == H2C ? "H2C" : "C2H"), engine->channel);
        return;
    }

    request = WdfDmaTransactionGetRequest(engine->dmaTransaction);
    if (!request)
    {
        engine->isReqPending = FALSE;

        WdfSpinLockRelease(engine->engineLock);

        TraceWarning(DBG_DMA, "%!FUNC!: WdfDmaTransactionGetRequest failed");
        return;
    }

    engine_status = EnginReadStatus(engine, TRUE);
    EngineStop(engine);

    //清除描述符缓冲
    desc = (DMA_DESCRIPTOR*)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
    RtlZeroMemory(desc, WdfCommonBufferGetLength(engine->descBuffer));

    engine->isReqPending = FALSE;
    WdfSpinLockRelease(engine->engineLock);

    //判断状态
    if (
        (engine_status & XDMA_BUSY_BIT) == 0 &&
        (engine_status & (XDMA_STAT_READ_ERROR | XDMA_STAT_DESCRIPTOR_ERROR)) == 0
        )
    {
        //正常完成
        BOOLEAN completes = FALSE;
        size_t bytes = 0;

        completes = WdfDmaTransactionDmaCompleted(engine->dmaTransaction, &status);
        bytes = WdfDmaTransactionGetBytesTransferred(engine->dmaTransaction);

        TraceInfo(DBG_DMA, "%!FUNC!: %s_%u completed, bytes=%Iu",
            (engine->dir == H2C ? "H2C" : "C2H"), engine->channel, bytes);

        if (completes)
        {
            KeSetEvent(&engine->completionEvent, IO_NO_INCREMENT, FALSE);
            WdfDmaTransactionRelease(engine->dmaTransaction);
            WdfRequestCompleteWithInformation(request, status, bytes);
        }
    }
    else
    {
        //错误
        TraceError(DBG_DMA, "%!FUNC!: %s_%u error engine_status=0x%08x",
            (engine->dir == H2C ? "H2C" : "C2H"), engine->channel, engine_status);

        WdfDmaTransactionDmaCompletedFinal(engine->dmaTransaction, 0, &status);
        KeSetEvent(&engine->completionEvent, IO_NO_INCREMENT, FALSE);
        WdfDmaTransactionRelease(engine->dmaTransaction);
        WdfRequestComplete(request, STATUS_INTERNAL_ERROR);
    }

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}