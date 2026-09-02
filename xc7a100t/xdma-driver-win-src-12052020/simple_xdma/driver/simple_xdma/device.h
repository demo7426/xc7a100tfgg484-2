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

#include "public.h"

typedef struct _BAR_INFO
{
    LARGE_INTEGER physical_address;     //物理地址
    ULONG length;                       //长度

    PVOID kernel_virtual_address;       //映射后的内核态虚拟地址
    PVOID user_virtual_address;         //映射后的用户态虚拟地址

    BOOLEAN is_valid;                   //该Bar当前是否有效；TRUE--有效；False--无效
}BAR_INFO, *PBAR_INFO;

typedef struct _DEVICE_CONTEXT
{
    BAR_INFO bar_infos[BAR_MAX_NUM];
    

}DEVICE_CONTEXT, * PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)    //通过 GetDeviceContext 函数即可获取设备上下文

//初始化设备上下文
VOID InitDevice_Context(PDEVICE_CONTEXT device_context);

//添加设备
NTSTATUS EVT_WDF_Driver_Device_Add(_In_ WDFDRIVER Driver, _Inout_ PWDFDEVICE_INIT DeviceInit);


#endif // !__DEVICE_H__
