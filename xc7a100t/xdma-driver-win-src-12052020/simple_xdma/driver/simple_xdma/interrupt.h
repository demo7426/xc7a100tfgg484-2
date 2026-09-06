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

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <ntddk.h>
#include <wdf.h>

// 中断寄存器结构（与 XDMA IP 核布局一致）
typedef struct {
    UINT32 identifier;
    UINT32 userIntEnable;
    UINT32 userIntEnableW1S;
    UINT32 userIntEnableW1C;
    UINT32 channelIntEnable;
    UINT32 channelIntEnableW1S;
    UINT32 channelIntEnableW1C;
    UINT32 reserved_1[9];
    UINT32 userIntRequest;
    UINT32 channelIntRequest;
    UINT32 userIntPending;
    UINT32 channelIntPending;
    UINT32 reserved_2[12];
    UINT32 userVector[4];
    UINT32 reserved_3[4];
    UINT32 channelVector[2];
} XDMA_IRQ_REGS;

typedef struct _INTERRUPT_CONTEXT
{
    volatile XDMA_IRQ_REGS* regs;
}INTERRUPT_CONTEXT, *PINTERRUPT_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(INTERRUPT_CONTEXT, GetInterruptContext)

//初始化中断
NTSTATUS SetupInterrupts(_In_ WDFDEVICE device, _In_ WDFCMRESLIST resources_raw, _In_ WDFCMRESLIST resources_translated, _In_ volatile XDMA_IRQ_REGS* regs);

#endif // !__INTERRUPT_H__
