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
#include "file.h"
#include "wdmguid.h"

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
        device_context->bar_infos[i].is_valid = FALSE;
    }
}

//获取真实的bar索引
static NTSTATUS GetRealBarIndex(IN WDFDEVICE wdfDevice, PHYSICAL_ADDRESS start, OUT PULONG bar_index) {

    BUS_INTERFACE_STANDARD pciBus = { 0 };
    NTSTATUS status = WdfFdoQueryForInterface(wdfDevice, &GUID_BUS_INTERFACE_STANDARD,
        (PINTERFACE)&pciBus, sizeof(BUS_INTERFACE_STANDARD), 1, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    PCI_COMMON_HEADER pciHeader = { 0 };
    ULONG bytesRead = pciBus.GetBusData(pciBus.Context, PCI_WHICHSPACE_CONFIG, &pciHeader, 0, PCI_COMMON_HDR_LENGTH);
    if (bytesRead != (ULONG)PCI_COMMON_HDR_LENGTH) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    status = STATUS_INVALID_DEVICE_REQUEST;

    for (ULONG i = 0; i < PCI_TYPE0_ADDRESSES; i++) {
        ULONG barVal = pciHeader.u.type0.BaseAddresses[i];
        if (barVal == 0) {
            continue;
        }
        if (barVal & 0x1) {
            continue;
        }
        BOOLEAN is64bit = (((barVal >> 1) & 0x3) == 0x2); // bits[2:1]==10
        BOOLEAN prefetchable = (barVal & 0x8) ? TRUE : FALSE;

        TraceInfo(DBG_INIT, "%s, bar[%u] raw=0x%x, addr=0x%x, %s-bit%s",
            __func__, i, barVal, barVal & 0xFFFFFFF0,
            is64bit ? "64" : "32",
            prefetchable ? ", prefetchable" : ", non-prefetchable");

        PHYSICAL_ADDRESS bar_addr = { .QuadPart = 0 };

        if (is64bit) {
            if (i + 1 >= PCI_TYPE0_ADDRESSES)       //防止内存越界
                break;

            ULONG bar_low = barVal & 0xFFFFFFF0;
            ULONG bar_high = pciHeader.u.type0.BaseAddresses[i + 1];

            bar_addr.QuadPart = (((ULONG64)bar_high << 32)) | bar_low;
        }
        else
        {
            bar_addr.QuadPart = barVal & 0xFFFFFFF0;
        }

        if (bar_addr.QuadPart == start.QuadPart)
        {
            *bar_index = i;
            status = STATUS_SUCCESS;
        }
  
        if (is64bit) {
            i++;            //比较完再跳过高 32 位
        }

    }

    return status;
}

//映射所有的Bar寄存器
static NTSTATUS MapBars(_In_ WDFDEVICE Device, _In_ WDFCMRESLIST ResourcesRaw, _In_ WDFCMRESLIST ResourcesTranslated)
{
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    TraceVerbose(DBG_INIT, "MapBars is enter.");

    NTSTATUS status = STATUS_SUCCESS;

    PDEVICE_CONTEXT ptDevice_Context = GetDeviceContext(Device);

    const ULONG ulCmReourceCount = WdfCmResourceListGetCount(ResourcesRaw);     //资源数量

    ULONG ulBarIndex = 0;

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
            //获取真实的bar索引
            ulBarIndex = 0;
            status = GetRealBarIndex(Device, ptResource->u.Memory.Start, &ulBarIndex);
            if (!NT_SUCCESS(status))
            {
                TraceError(DBG_INIT, "%!FUNC!: GetRealBarIndex failed: %!STATUS!", status);
                return status;
            }

            if (ulBarIndex < sizeof(ptDevice_Context->bar_infos) / sizeof(ptDevice_Context->bar_infos[0]))
            {
                ptDevice_Context->bar_infos[ulBarIndex].length = ptResource->u.Memory.Length;
                ptDevice_Context->bar_infos[ulBarIndex].physical_address = ptResource->u.Memory.Start;

                ptDevice_Context->bar_infos[ulBarIndex].kernel_virtual_address = MmMapIoSpace(ptResource->u.Memory.Start, ptResource->u.Memory.Length, MmNonCached);
                if (ptDevice_Context->bar_infos[ulBarIndex].kernel_virtual_address == NULL) {
                    TraceError(DBG_INIT, "MmMapIoSpace returned NULL! for BAR%u", ulBarIndex);
                    return STATUS_DEVICE_CONFIGURATION_ERROR;
                }

                ptDevice_Context->bar_infos[ulBarIndex].is_valid = TRUE;

                TraceInfo(DBG_INIT, "MM BAR %d (addr:0x%I64x, length:0x%x) mapped at 0x%08p",
                    ulBarIndex, ptResource->u.Memory.Start.QuadPart,
                    ptResource->u.Memory.Length, ptDevice_Context->bar_infos[ulBarIndex].kernel_virtual_address);
            }
            else
            {
                TraceInfo(DBG_INIT, "MapBars ulBarIndex = %u.", ulBarIndex);
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

    WdfDeviceInitSetIoType(device_init, WdfDeviceIoDirect);     //设置 WDFDEVICE_INIT 的IO方式;主要是ReadFile，WriteFile

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&tWDF_Object_Attributes, DEVICE_CONTEXT);       //增加设备上下文对象的创建属性
    

    // Register file object call-backs
    WDF_OBJECT_ATTRIBUTES fileAttributes;
    WDF_FILEOBJECT_CONFIG fileConfig;
    WDF_FILEOBJECT_CONFIG_INIT(&fileConfig, EvtDeviceFileCreate, EvtFileClose, EvtFileCleanup);
    WDF_OBJECT_ATTRIBUTES_INIT(&fileAttributes);
    fileAttributes.SynchronizationScope = WdfSynchronizationScopeNone;
    WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&fileAttributes, FILE_CONTEXT);
    WdfDeviceInitSetFileObjectConfig(device_init, &fileConfig, &fileAttributes);
    
    
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

