/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	driver.c
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

#include "driver.h"
#include "device.h"

#include "trace.h"

#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro 
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "driver.tmh"
#endif

/// <summary>
/// 初始化注册表参数
/// </summary>
/// <param name="driver_object"></param>
/// <returns></returns>
static NTSTATUS InitRegistryParameter(IN PDRIVER_OBJECT driver_object);

/// <summary>
/// 驱动卸载
/// </summary>
/// <param name="driver_object">驱动对象</param>
/// <returns></returns>
VOID DriverUnload(IN PDRIVER_OBJECT driver_object)
{
    TraceVerbose(DBG_INIT, "DriverUnload is end.");

    WPP_CLEANUP(driver_object);
}

/// <summary>
/// 驱动加载
/// </summary>
/// <param name="driver_object">驱动对象</param>
/// <param name="register_path">注册表路径</param>
/// <returns></returns>
NTSTATUS DriverEntry(IN PDRIVER_OBJECT driver_object, IN PUNICODE_STRING register_path)
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG tWDF_Driver_Config = { 0 };
    WDF_OBJECT_ATTRIBUTES tWDF_Object_Attributes = { 0 };
    WDFDRIVER tWDFDriver = NULL;

    PDRIVER_CONTEXT ptDriver_Context = NULL;

    const char* const dataTimeStr = "Built " __DATE__ ", " __TIME__ ".";

    WPP_INIT_TRACING(driver_object, register_path);

    TraceVerbose(DBG_INIT, "DriverEntry is start.");

    WDF_DRIVER_CONFIG_INIT(&tWDF_Driver_Config, EVT_WDF_Driver_Device_Add);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&tWDF_Object_Attributes, DRIVER_CONTEXT);      

    //创建框架驱动程序对象
    status = WdfDriverCreate(driver_object, register_path, &tWDF_Object_Attributes, &tWDF_Driver_Config, &tWDFDriver);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "WdfDriverCreate is error.");

        WPP_CLEANUP(driver_object);
        return status;
    }

    ptDriver_Context = GetDriverContext(tWDFDriver);
    if (!ptDriver_Context)
    {
        TraceError(DBG_INIT, "GetDriverContext is error.");

        WPP_CLEANUP(driver_object);
        return STATUS_INVALID_PARAMETER;
    }

    ANSI_STRING ansi;
    UNICODE_STRING uni;

    RtlInitAnsiString(&ansi, dataTimeStr);
    RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);

    if (uni.Length + sizeof(WCHAR) <= sizeof ptDriver_Context->versions)        //防止内存越界
    {
        RtlCopyMemory(ptDriver_Context->versions, uni.Buffer, uni.Length + sizeof(WCHAR));
    }
    else
    {
        TraceError(DBG_INIT, "uni.Length = %u��sizeof ptDriver_Context->versions = %u.", uni.Length, sizeof ptDriver_Context->versions);

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlFreeUnicodeString(&uni);

    if (NT_SUCCESS(status))
        status = InitRegistryParameter(driver_object);

    driver_object->DriverUnload = DriverUnload;

    TraceVerbose(DBG_INIT, "DriverEntry is end.");

    return status;
}

static NTSTATUS InitRegistryParameter(IN PDRIVER_OBJECT driver_object)
{
    UNREFERENCED_PARAMETER(driver_object);

    NTSTATUS status = STATUS_SUCCESS;

    return status;
}