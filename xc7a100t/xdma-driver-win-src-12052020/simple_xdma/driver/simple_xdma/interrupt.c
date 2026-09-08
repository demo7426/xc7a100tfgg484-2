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
#include <wdmguid.h>

#ifdef DBG
#include "interrupt.tmh"
#endif

// 构建中断向量寄存器值（每个字节对应一个通道的 MSI/MSI-X 向量号）
static UINT32 BuildVectorReg(UINT32 a, UINT32 b, UINT32 c, UINT32 d)
{
    UINT32 reg_val = 0;
    reg_val |= (a & 0x1f) << 0;
    reg_val |= (b & 0x1f) << 8;
    reg_val |= (c & 0x1f) << 16;
    reg_val |= (d & 0x1f) << 24;
    return reg_val;
}

//使能中断
static NTSTATUS EvtInterruptEnable(WDFINTERRUPT interrupt, WDFDEVICE device)
{
    UNREFERENCED_PARAMETER(device);

    TraceVerbose(DBG_IRQ, "%!FUNC! is enter.");

    PINTERRUPT_CONTEXT ptInterrupt_Context = GetInterruptContext(interrupt);
    if (ptInterrupt_Context)
    {
        ptInterrupt_Context->regs->userIntEnableW1S = 0xffffffff;
        ptInterrupt_Context->regs->channelIntEnableW1S = 0xffffffff;
        TraceInfo(DBG_IRQ, "%!FUNC!: User and Channel interrupts enabled");
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

    UINT32 chanIrq = ptInterrupt_Context->regs->channelIntRequest;      //4路 DMA 读写通道中断
    UINT32 userIrq = ptInterrupt_Context->regs->userIntRequest;         //FPGA 逻辑层额外自定义的用户中断（User Interrupt）

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
        //处理自定义用户中断
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

        //创建中断资源，并注册isr、dpc函数
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

        // 编程中断向量寄存器：告诉 XDMA 硬件每个通道/user 中断使用哪个 MSI/MSI-X 向量
        // MSI/MSI-X 时 vectorValue = 0；线中断时需要获取 PCI 中断引脚号
        UINT32 vectorValue = 0;
        if (!(ptResourceTranslated->Flags & CM_RESOURCE_INTERRUPT_MESSAGE))
        {
            // 线中断：获取 PCI 中断引脚 (A=1→0, B=2→1, C=3→2, D=4→3)
            BUS_INTERFACE_STANDARD pciBus = { 0 };
            NTSTATUS busStatus = WdfFdoQueryForInterface(device, &GUID_BUS_INTERFACE_STANDARD,
                (PINTERFACE)&pciBus, sizeof(BUS_INTERFACE_STANDARD), 1, NULL);
            if (NT_SUCCESS(busStatus))
            {
                PCI_COMMON_HEADER pciHeader = { 0 };
                ULONG bytesRead = pciBus.GetBusData(pciBus.Context, PCI_WHICHSPACE_CONFIG,
                    &pciHeader, 0, PCI_COMMON_HDR_LENGTH);
                if (bytesRead == PCI_COMMON_HDR_LENGTH)
                {
                    // Windows: INTA=1, INTB=2, INTC=3, INTD=4
                    // XDMA:    INTA=0, INTB=1, INTC=2, INTD=3
                    vectorValue = pciHeader.u.type0.InterruptPin - 1;
                    TraceInfo(DBG_INIT, "%!FUNC!: Line interrupt pin=%u, vectorValue=%u",
                        pciHeader.u.type0.InterruptPin, vectorValue);
                }
            }
        }

        TraceInfo(DBG_INIT, "%!FUNC!: Programming interrupt vectors with value=%u", vectorValue);

        // 编程 userVector[0..3]：每个寄存器 4 个字节，共支持 16 个 user 中断
        regs->userVector[0] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);
        regs->userVector[1] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);
        regs->userVector[2] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);
        regs->userVector[3] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);

        // 编程 channelVector[0..1]：每个寄存器 4 个字节，共支持 8 个 channel 中断
        regs->channelVector[0] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);
        regs->channelVector[1] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);

        status = STATUS_SUCCESS;

        break;      //使用第一个中断即可
    }

    TraceVerbose(DBG_INIT, "%!FUNC! is end.");

    return status;
}