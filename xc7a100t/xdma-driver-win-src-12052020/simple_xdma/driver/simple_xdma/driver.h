/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	driver.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.31
描  述: 驱动入口文件
备  注:
修改记录:

  1.  日期: 2026.08.31
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <ntddk.h>
#include <wdf.h>

typedef struct _DRIVER_CONTEXT
{
    WCHAR versions[128];     //版本信息
}DRIVER_CONTEXT, *PDRIVER_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DRIVER_CONTEXT, GetDriverContext)    //通过 GetDriverContext 函数即可获取驱动上下文


#endif // !__DRIVER_H__