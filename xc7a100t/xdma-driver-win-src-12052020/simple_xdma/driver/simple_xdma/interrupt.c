/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	interrupt.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.09.06
描  述: 中断文件
备  注:
修改记录:

  1.  日期: 2026.09.06
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include "interrupt.h"
#include "trace.h"
#include "dma_engine.h"
#include "device.h"

#ifdef DBG
#include "interrupt.tmh"
#endif

//使能中断
static NTSTATUS EvtInterruptEnable(WDFINTERRUPT interrupt, WDFDEVICE device)
{
    UNREFERENCED_PARAMETER(device);

    TraceVerbose(DBG_IRQ, "%!FUNC! is enter.");

    PINTERRUPT_CONTEXT ptInterrupt_Context = GetInterruptContext(interrupt);
    if (ptInterrupt_Context)
    {
        ptInterrupt_Context->regs->userIntEnableW1S = 0xffffffff;
        //ptInterrupt_Context->regs->channelIntEnableW1S = 0xffffffff;
        TraceInfo(DBG_IRQ, "%!FUNC!: User interrupt enabled");
    }

    TraceVerbose(DBG_IRQ, "%!FUNC! is end.");

    return STATUS_SUCCESS;
}

//取消中断使能
static NTSTATUS EvtInterruptDisable(WDFINTERRUPT interrupt, WDFDEVICE device)
{
    UNREFERENCED_PARAMETER(device);

    TraceVerbose(DBG_IRQ, "%!FUNC! is enter.");

    PINTERRUPT_CONTEXT ptInterrupt_Context = GetInterruptContext(interrupt);
    if (ptInterrupt_Context)
    {
        ptInterrupt_Context->regs->userIntEnableW1C = 0xffffffff;
        TraceInfo(DBG_IRQ, "%!FUNC!: User interrupt disabled");
    }

    TraceVerbose(DBG_IRQ, "%!FUNC! is end.");

    return STATUS_SUCCESS;
}

//isr函数
BOOLEAN EvtInterruptIsr(WDFINTERRUPT interrupt, ULONG message_id)
{
    TraceVerbose(DBG_IRQ, "%!FUNC! is enter.");

    PINTERRUPT_CONTEXT ptInterrupt_Context = GetInterruptContext(interrupt);
    if (!ptInterrupt_Context)
    {
        TraceError(DBG_IRQ, "%!FUNC!: GetInterruptContext failed: ");
        return FALSE;
    }

    UINT32 chanIrq = ptInterrupt_Context->regs->channelIntRequest;      //4 路 DMA 引擎通道中断
    UINT32 userIrq = ptInterrupt_Context->regs->userIntRequest;         //FPGA 逻辑留给开发者自定义的用户中断（User Interrupt）

    TraceInfo(DBG_IRQ, "%!FUNC!: message_id = %u, channelIntRequest = 0x%08x, userIntRequest = 0x%08x", message_id, chanIrq, userIrq);

    if (chanIrq == 0 && userIrq == 0)
    {
        TraceWarning(DBG_IRQ, "%!FUNC!: Suprious interrupt");
        return FALSE;
    }

    //关闭已触发的中断

    if (chanIrq)
    {
        ptInterrupt_Context->channelIrqPending = chanIrq;
        ptInterrupt_Context->regs->channelIntEnableW1C = chanIrq;       
    }

    if (userIrq)
    {
        ptInterrupt_Context->userIrqPending = userIrq;
        ptInterrupt_Context->regs->userIntEnableW1C = userIrq;
    }

    TraceInfo(DBG_IRQ, "%!FUNC!: WdfInterruptQueueDpcForIsr");

    TraceVerbose(DBG_IRQ, "%!FUNC! is end.");

    return WdfInterruptQueueDpcForIsr(interrupt);
}

//Dpc函数
static VOID EvtInterruptDpc(WDFINTERRUPT interrupt, WDFDEVICE device)
{
    UNREFERENCED_PARAMETER(device);

    TraceVerbose(DBG_IRQ, "%!FUNC! is enter.");

    PINTERRUPT_CONTEXT ptInterrupt_Context = GetInterruptContext(interrupt);
    if (!ptInterrupt_Context)
    {
        TraceError(DBG_IRQ, "%!FUNC!: GetInterruptContext failed: ");
        return;
    }

    if (ptInterrupt_Context->channelIrqPending)
    {
        EngineProcessChannelInterrupt(ptInterrupt_Context->deviceContext);
    }

    if (ptInterrupt_Context->userIrqPending)
    {
        //处理自定义的用户中断
    }

    //重新使能中断
    WdfInterruptAcquireLock(interrupt);

    if (ptInterrupt_Context->channelIrqPending)
    {
        ptInterrupt_Context->regs->channelIntEnableW1S = ptInterrupt_Context->channelIrqPending;
        ptInterrupt_Context->channelIrqPending = 0;
    }
    
    if (ptInterrupt_Context->userIrqPending)
    {
        ptInterrupt_Context->regs->userIntEnableW1S = ptInterrupt_Context->userIrqPending;
        ptInterrupt_Context->userIrqPending = 0;
    }   

    WdfInterruptReleaseLock(interrupt);

    TraceInfo(DBG_IRQ, "%!FUNC!: user interrupt occured");

    TraceVerbose(DBG_IRQ, "%!FUNC! is end.");
}

NTSTATUS SetupInterrupts(_In_ WDFDEVICE device, _In_ WDFCMRESLIST resources_raw, _In_ WDFCMRESLIST resources_translated, _In_ volatile XDMA_IRQ_REGS* regs)
{
    TraceVerbose(DBG_INIT, "%!FUNC! is enter.");

    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
    const ULONG ulCmResourceCount = WdfCmResourceListGetCount(resources_raw);     //资源数量

    for (ULONG index = 0; index < ulCmResourceCount; ++index)
    {
        PCM_PARTIAL_RESOURCE_DESCRIPTOR ptResourceRaw = WdfCmResourceListGetDescriptor(resources_raw, index);
        if (!ptResourceRaw)
        {
            TraceError(DBG_INIT, "%!FUNC!: WdfCmResourceListGetDescriptor failed: %!STATUS!, index = %u", status, index);
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }
        
        PCM_PARTIAL_RESOURCE_DESCRIPTOR ptResourceTranslated = WdfCmResourceListGetDescriptor(resources_translated, index);
        if (!ptResourceTranslated)
        {
            TraceError(DBG_INIT, "%!FUNC!: WdfCmResourceListGetDescriptor failed: %!STATUS!, index = %u", status, index);
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }

        if (ptResourceTranslated->Type != CmResourceTypeInterrupt)
            continue;

        //设置中断资源、isr、dpc函数
        WDF_INTERRUPT_CONFIG tWDF_Interrupt_Config;
        WDF_INTERRUPT_CONFIG_INIT(&tWDF_Interrupt_Config, EvtInterruptIsr, EvtInterruptDpc);
        tWDF_Interrupt_Config.InterruptRaw = ptResourceRaw;
        tWDF_Interrupt_Config.InterruptTranslated = ptResourceTranslated;
        tWDF_Interrupt_Config.EvtInterruptEnable = EvtInterruptEnable;
        tWDF_Interrupt_Config.EvtInterruptDisable = EvtInterruptDisable;

        WDF_OBJECT_ATTRIBUTES attributes;
        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, INTERRUPT_CONTEXT);

        WDFINTERRUPT tWDFInterrupt = NULL;
        status = WdfInterruptCreate(device, &tWDF_Interrupt_Config, &attributes, &tWDFInterrupt);
        if (!NT_SUCCESS(status))
        {
            TraceError(DBG_INIT, "%!FUNC!: WdfInterruptCreate failed: %!STATUS!", status);
            return status;
        }

        PINTERRUPT_CONTEXT ptInterrupt_Context = GetInterruptContext(tWDFInterrupt);
        if (ptInterrupt_Context)
        {
            ptInterrupt_Context->regs = regs;
            ptInterrupt_Context->deviceContext = GetDeviceContext(device);
        }

        status = STATUS_SUCCESS;

        break;      //使用第一个中断即可
    }

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");

    return status;
}
