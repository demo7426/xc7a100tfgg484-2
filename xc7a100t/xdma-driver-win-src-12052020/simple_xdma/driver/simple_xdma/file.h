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

typedef struct _FILE_CONTEXT
{
    DWORD32 reserve;        //预留
}FILE_CONTEXT, * PFILE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FILE_CONTEXT, GetFileContext)



#endif // !__FILE_H__