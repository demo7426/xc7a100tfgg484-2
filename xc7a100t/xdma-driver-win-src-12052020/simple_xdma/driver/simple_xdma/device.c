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


#include "device.h"

#include "trace.h"

#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro 
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "device.tmh"
#endif

NTSTATUS EVT_WDF_Driver_Device_Add(_In_ WDFDRIVER driver, _Inout_ PWDFDEVICE_INIT device_init)
{
    UNREFERENCED_PARAMETER(driver);

    NTSTATUS status = STATUS_SUCCESS;
    WDFDEVICE tWDFDevice = NULL;
    WDF_OBJECT_ATTRIBUTES tWDF_Object_Attributes = { 0 };

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&tWDF_Object_Attributes, DEVICE_CONTEXT);       //增加设备上下文对象的创建属性

    /*
    WdfSynchronizationScopeDevice  → 框架持有 WDFDEVICE 关联的 spinlock
    WdfSynchronizationScopeQueue   → 框架持有 WDFQUEUE 关联的 spinlock
    WdfSynchronizationScopeNone    → 框架不帮你加锁
    */
    tWDF_Object_Attributes.SynchronizationScope = WdfSynchronizationScopeNone;              //在队列里面加锁就可以了       

    //创建设备文件
    status = WdfDeviceCreate(&device_init, &tWDF_Object_Attributes, &tWDFDevice);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "WdfDeviceCreate failed: %!STATUS!", status);
        return status;
    }

    return status;
}



