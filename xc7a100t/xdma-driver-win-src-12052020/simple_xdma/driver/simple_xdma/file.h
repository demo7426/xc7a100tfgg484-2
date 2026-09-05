/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	file.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.09.01
描  述: 
备  注:
修改记录:

  1.  日期: 2026.09.01
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#ifndef __FILE_H__
#define __FILE_H__

#include <ntddk.h>
#include <wdf.h>

#include "simple_xdma_public.h"

enum FILE_TYPE
{
    FILE_TYPE_NONE = 0,

    FILE_TYPE_USER = 1,
    FILE_TYPE_CONTROL,
    FILE_TYPE_BYPASS_0,
    FILE_TYPE_BYPASS_1,
    FILE_TYPE_BYPASS_2,
    FILE_TYPE_BYPASS_3,

    FILE_TYPE_H2C,
    FILE_TYPE_C2H,
};

typedef struct _FILE_CONTEXT
{
    enum FILE_TYPE file_type;
    const WCHAR* file_name;
    ULONG channel;

    XDMA_BAR_INFO bar_infos[BAR_MAX_NUM];         //bar映射后的用户态虚拟地址
    PMDL mdls[BAR_MAX_NUM];
}FILE_CONTEXT, * PFILE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FILE_CONTEXT, GetFileContext)

VOID EVT_WDF_IO_IN_Caller_Context(_In_ WDFDEVICE Device, _In_ WDFREQUEST Request);

VOID EvtDeviceFileCreate(_In_ WDFDEVICE Device, _In_ WDFREQUEST Request, _In_ WDFFILEOBJECT FileObject);

VOID EvtFileClose(_In_ WDFFILEOBJECT FileObject);

VOID EvtFileCleanup(_In_ WDFFILEOBJECT FileObject);

#endif // !__FILE_H__