/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	queue.h
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

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <ntddk.h>
#include <wdf.h>

typedef struct _QUEUE_CONTEXT
{
    DWORD32 reserve;        //预留
}QUEUE_CONTEXT, * PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, GetQueueContext)

VOID EVT_WDF_IO_Queue_IO_Device_Control(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_ size_t OutputBufferLength, _In_ size_t InputBufferLength, _In_ ULONG IoControlCode);


#endif // !__QUEUE_H__