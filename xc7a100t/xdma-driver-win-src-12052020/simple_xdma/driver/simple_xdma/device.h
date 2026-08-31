/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	device.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.31
描  述: 设备文件
备  注:
修改记录:

  1.  日期: 2026.08.31
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <ntddk.h>
#include <wdf.h>

typedef struct _DEVICE_CONTEXT
{
    DWORD32 reserve;        //预留
}DEVICE_CONTEXT, * PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)    //通过 GetDeviceContext 函数即可获取设备上下文

//添加设备
NTSTATUS EVT_WDF_Driver_Device_Add(_In_ WDFDRIVER Driver, _Inout_ PWDFDEVICE_INIT DeviceInit);



#endif // !__DEVICE_H__
