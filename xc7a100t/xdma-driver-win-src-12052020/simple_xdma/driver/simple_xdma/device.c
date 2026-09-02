/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	device.c
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
#include "queue.h"

#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro 
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "device.tmh"
#endif

VOID InitDevice_Context(PDEVICE_CONTEXT device_context)
{
    if (!device_context)
        return;

    for (ULONG i = 0; i < sizeof(device_context->bar_infos) / sizeof(device_context->bar_infos[0]); i++)
    {
        device_context->bar_infos[i].physical_address.QuadPart = 0;
        device_context->bar_infos[i].length = 0;

        device_context->bar_infos[i].kernel_virtual_address = NULL;
        device_context->bar_infos[i].user_virtual_address = NULL;
        device_context->bar_infos[i].is_valid = FALSE;
    }
}

//映射所有的Bar寄存器
static NTSTATUS MapBars(_In_ WDFDEVICE Device, _In_ WDFCMRESLIST ResourcesRaw, _In_ WDFCMRESLIST ResourcesTranslated)
{
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    TraceVerbose(DBG_INIT, "MapBars is enter.");

    NTSTATUS status = STATUS_SUCCESS;

    PDEVICE_CONTEXT ptDevice_Context = GetDeviceContext(Device);

    const ULONG ulCmReourceCount = WdfCmResourceListGetCount(ResourcesRaw);     //资源数量

    ULONG ulBarNum = 0;                   //bar当前的数量

    InitDevice_Context(ptDevice_Context);

    for (ULONG index = 0; index < ulCmReourceCount; ++index)
    {
        PCM_PARTIAL_RESOURCE_DESCRIPTOR ptResource = WdfCmResourceListGetDescriptor(ResourcesRaw, index);
        if (!ptResource)
        {
            TraceError(DBG_INIT, "WdfCmResourceListGetDescriptor failed: %!STATUS!, index = %u", status, index);
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }

        switch (ptResource->Type)
        {
        case CmResourceTypeMemory:
        {
            if (ulBarNum < sizeof(ptDevice_Context->bar_infos) / sizeof(ptDevice_Context->bar_infos[0]))
            {
                ptDevice_Context->bar_infos[ulBarNum].length = ptResource->u.Memory.Length;
                ptDevice_Context->bar_infos[ulBarNum].physical_address = ptResource->u.Memory.Start;

                ptDevice_Context->bar_infos[ulBarNum].kernel_virtual_address = MmMapIoSpace(ptResource->u.Memory.Start, ptResource->u.Memory.Length, MmNonCached);
                if (ptDevice_Context->bar_infos[ulBarNum].kernel_virtual_address == NULL) {
                    TraceError(DBG_INIT, "MmMapIoSpace returned NULL! for BAR%u", ulBarNum);
                    return STATUS_DEVICE_CONFIGURATION_ERROR;
                }

                ptDevice_Context->bar_infos[ulBarNum].is_valid = TRUE;

                TraceInfo(DBG_INIT, "MM BAR %d (addr:0x%I64x, length:0x%x) mapped at 0x%08p",
                    ulBarNum, ptResource->u.Memory.Start.QuadPart,
                    ptResource->u.Memory.Length, ptDevice_Context->bar_infos[ulBarNum].kernel_virtual_address);

                ulBarNum++;
            }
            else
            {
                TraceInfo(DBG_INIT, "MapBars ulBarNum = %u.", ulBarNum);
            }
        }
            break;
        default:
            break;
        }
    }


    TraceVerbose(DBG_INIT, "MapBars is end.");

    return status;
}

static NTSTATUS EVT_WDF_Device_Prepare_Hardware(_In_ WDFDEVICE Device, _In_ WDFCMRESLIST ResourcesRaw, _In_ WDFCMRESLIST ResourcesTranslated)
{
    TraceVerbose(DBG_INIT, "EVT_WDF_Device_Prepare_Hardware is enter.");

    NTSTATUS status = STATUS_SUCCESS;

    status = MapBars(Device, ResourcesRaw, ResourcesTranslated);

    TraceVerbose(DBG_INIT, "EVT_WDF_Device_Prepare_Hardware is enter.");

    return status;
}

static NTSTATUS EVT_WDF_Device_Relase_Hardware(_In_ WDFDEVICE Device, _In_ WDFCMRESLIST ResourcesTranslated)
{
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    TraceInfo(DBG_INIT, "EVT_WDF_Device_Relase_Hardware is enter.");

    PDEVICE_CONTEXT ptDevice_Context = GetDeviceContext(Device);
    for (ULONG i = 0; i < sizeof(ptDevice_Context->bar_infos) / sizeof(ptDevice_Context->bar_infos[0]); i++)
    {
        if (ptDevice_Context->bar_infos[i].is_valid == TRUE && ptDevice_Context->bar_infos[i].kernel_virtual_address != NULL)
        {
            MmUnmapIoSpace(ptDevice_Context->bar_infos[i].kernel_virtual_address, ptDevice_Context->bar_infos[i].length);
            
            ptDevice_Context->bar_infos[i].is_valid = FALSE;
        }

    }

    return STATUS_SUCCESS;
}

static VOID EVT_WDF_IO_IN_Caller_Context(_In_ WDFDEVICE Device, _In_ WDFREQUEST Request)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(Request);

    TraceInfo(DBG_INIT, "EVT_WDF_IO_IN_Caller_Context is enter.");

    return;
}

NTSTATUS EVT_WDF_Driver_Device_Add(_In_ WDFDRIVER driver, _Inout_ PWDFDEVICE_INIT device_init)
{
    UNREFERENCED_PARAMETER(driver);

    TraceVerbose(DBG_INIT, "EVT_WDF_Driver_Device_Add is enter.");

    NTSTATUS status = STATUS_SUCCESS;
    WDFDEVICE tWDFDevice = NULL;
    WDF_OBJECT_ATTRIBUTES tWDF_Object_Attributes = { 0 };

    WDF_PNPPOWER_EVENT_CALLBACKS tWDFPNPPowerCallBacks = { 0 };

    WDF_IO_QUEUE_CONFIG tWDF_IO_Queue_Config = { 0 };
    WDFQUEUE tWDFQueue = { 0 };

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&tWDF_Object_Attributes, DEVICE_CONTEXT);       //增加设备上下文对象的创建属性
    /*
    WdfSynchronizationScopeDevice  → 框架持有 WDFDEVICE 关联的 spinlock
    WdfSynchronizationScopeQueue   → 框架持有 WDFQUEUE 关联的 spinlock
    WdfSynchronizationScopeNone    → 框架不帮你加锁
    */
    tWDF_Object_Attributes.SynchronizationScope = WdfSynchronizationScopeNone;              //在队列里面加锁就可以了       

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&tWDFPNPPowerCallBacks);

    //PNP hardware
    tWDFPNPPowerCallBacks.EvtDevicePrepareHardware = EVT_WDF_Device_Prepare_Hardware;
    tWDFPNPPowerCallBacks.EvtDeviceReleaseHardware = EVT_WDF_Device_Relase_Hardware;

    WdfDeviceInitSetPnpPowerEventCallbacks(device_init, &tWDFPNPPowerCallBacks);

    WdfDeviceInitSetIoInCallerContextCallback(device_init, EVT_WDF_IO_IN_Caller_Context);

    //创建设备文件
    status = WdfDeviceCreate(&device_init, &tWDF_Object_Attributes, &tWDFDevice);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "WdfDeviceCreate failed: %!STATUS!", status);
        return status;
    }

    //必须使用WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE函数初始化，否则会导致应用层IO调用失败
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&tWDF_IO_Queue_Config, WdfIoQueueDispatchSequential);
    tWDF_IO_Queue_Config.EvtIoDeviceControl = EVT_WDF_IO_Queue_IO_Device_Control;

    status = WdfIoQueueCreate(tWDFDevice, &tWDF_IO_Queue_Config, WDF_NO_OBJECT_ATTRIBUTES, &tWDFQueue);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "WdfIoQueueCreate failed: %!STATUS!", status);
        return status;
    }

    status = WdfDeviceCreateDeviceInterface(tWDFDevice, &GUID_DEVINTERFACE_XDMA, NULL);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "WdfDeviceCreateDeviceInterface failed: %!STATUS!", status);
        return status;
    }

    TraceVerbose(DBG_INIT, "EVT_WDF_Driver_Device_Add is end.");

    return status;
}

