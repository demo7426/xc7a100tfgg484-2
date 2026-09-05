/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	file.c
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

#include "file.h"
#include "trace.h"
#include "device.h"

#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro 
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "file.tmh"
#endif

static VOID GetFileType(_In_ PUNICODE_STRING file_name, _Out_ PFILE_CONTEXT file_context)
{
    if (file_context == NULL)
    {
        TraceError(DBG_INIT, "GetFileType input parameter is error.");
        return;
    }

    static FILE_CONTEXT tFileName_FileType_Infos[] = {
        {.file_type = FILE_TYPE_USER, .file_name = XDMA_FILE_USER, .channel = 0 },
        {.file_type = FILE_TYPE_CONTROL, .file_name = XDMA_FILE_CONTROL, .channel = 0 },
        {.file_type = FILE_TYPE_BYPASS_0, .file_name = XDMA_FILE_BYPASS_0, .channel = 0 },
        {.file_type = FILE_TYPE_BYPASS_1, .file_name = XDMA_FILE_BYPASS_1, .channel = 0 },
        {.file_type = FILE_TYPE_BYPASS_2, .file_name = XDMA_FILE_BYPASS_2, .channel = 0 },
        {.file_type = FILE_TYPE_BYPASS_3, .file_name = XDMA_FILE_BYPASS_3, .channel = 0 },
        
        {.file_type = FILE_TYPE_H2C, .file_name = XDMA_FILE_H2C_0, .channel = 0 },
        {.file_type = FILE_TYPE_H2C, .file_name = XDMA_FILE_H2C_1, .channel = 1 },
        {.file_type = FILE_TYPE_H2C, .file_name = XDMA_FILE_H2C_2, .channel = 2 },
        {.file_type = FILE_TYPE_H2C, .file_name = XDMA_FILE_H2C_3, .channel = 3 },
                      
        {.file_type = FILE_TYPE_C2H, .file_name = XDMA_FILE_C2H_0, .channel = 0 },
        {.file_type = FILE_TYPE_C2H, .file_name = XDMA_FILE_C2H_1, .channel = 1 },
        {.file_type = FILE_TYPE_C2H, .file_name = XDMA_FILE_C2H_2, .channel = 2 },
        {.file_type = FILE_TYPE_C2H, .file_name = XDMA_FILE_C2H_3, .channel = 3 },

    };

    file_context->file_type = FILE_TYPE_NONE;

    for (ULONG i = 0; i < sizeof(tFileName_FileType_Infos) / sizeof(tFileName_FileType_Infos[0]); i++)
    {
        if (wcscmp(file_name->Buffer, tFileName_FileType_Infos[i].file_name) == 0)
        {
            *file_context = tFileName_FileType_Infos[i];
        }

    }
}

static NTSTATUS IoCtrlMapBarsToUser(_In_ WDFDEVICE Device, _In_ WDFREQUEST Request, _In_ ULONG barIndex)
{
    NTSTATUS status = STATUS_SUCCESS;
    PUCHAR buffer;
    size_t totalLength;
    PXDMA_BAR_INFO xbar_info;

    PDEVICE_CONTEXT ptDevice_Context = GetDeviceContext(Device);
    PFILE_CONTEXT ptFile_Context = GetFileContext(WdfRequestGetFileObject(Request));

    status = WdfRequestRetrieveOutputBuffer(Request, sizeof(UCHAR), &buffer, &totalLength);
    if (!NT_SUCCESS(status)) {
        TraceError(DBG_IO, "%!FUNC!: Retrieve Buffer Failed - Length: %Iu", totalLength);
        return status;
    }

    if (buffer == NULL) {
        TraceError(DBG_IO, "%!FUNC!: Buffer is NULL");
        return STATUS_NOT_SUPPORTED;
    }

    if (totalLength != sizeof(XDMA_BAR_INFO)) {
        TraceError(DBG_IO, "%!FUNC!: Input data not equal to XDMA Keyhole Struct");
        return STATUS_NOT_SUPPORTED;
    }

    xbar_info = (PXDMA_BAR_INFO)buffer;

    if (barIndex >= sizeof ptFile_Context->mdls / sizeof ptFile_Context->mdls[0])
    {
        TraceError(DBG_IO, "%!FUNC!: barIndex = %u", barIndex);
        return STATUS_NOT_SUPPORTED;
    }

    if (!ptDevice_Context->bar_infos[barIndex].is_valid) {
        TraceError(DBG_IO, "%!FUNC!: BAR %u is not valid", barIndex);
        return STATUS_DEVICE_NOT_READY;
    }

    ptFile_Context->mdls[barIndex] = IoAllocateMdl(ptDevice_Context->bar_infos[barIndex].kernel_virtual_address, ptDevice_Context->bar_infos[barIndex].length, FALSE, FALSE, NULL);
    if (!ptFile_Context->mdls[barIndex]) {
        TraceError(DBG_IO, "%!FUNC!: Bad MDL allocation");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MmBuildMdlForNonPagedPool(ptFile_Context->mdls[barIndex]);

    try
    {
        ptFile_Context->bar_infos[barIndex].bar_user_virtual_address = MmMapLockedPagesSpecifyCache(
            ptFile_Context->mdls[barIndex], 
            UserMode, 
            MmNonCached,
            NULL, 
            FALSE, 
            NormalPagePriority | MdlMappingNoExecute
        );
        if (!ptFile_Context->bar_infos[barIndex].bar_user_virtual_address)
        {
            TraceError(DBG_IO, "%!FUNC! Bad Mapping");

            IoFreeMdl(ptFile_Context->mdls[barIndex]);
            ptFile_Context->mdls[barIndex] = NULL;

            return STATUS_INSUFFICIENT_RESOURCES;
        }

    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        TraceError(DBG_IO, "%!FUNC! Bad Mapping");

        IoFreeMdl(ptFile_Context->mdls[barIndex]);
        ptFile_Context->mdls[barIndex] = NULL;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ptFile_Context->bar_infos[barIndex].bar_length = ptDevice_Context->bar_infos[barIndex].length;

    xbar_info->bar_user_virtual_address = ptFile_Context->bar_infos[barIndex].bar_user_virtual_address;
    xbar_info->bar_length = ptFile_Context->bar_infos[barIndex].bar_length;

    return status;
}

VOID EVT_WDF_IO_IN_Caller_Context(_In_ WDFDEVICE Device, _In_ WDFREQUEST Request)
{
    TraceInfo(DBG_INIT, "%!FUNC! is enter.");

    NTSTATUS status = STATUS_SUCCESS;
    WDF_REQUEST_PARAMETERS params;
    PFILE_CONTEXT ptFile_Context = GetFileContext(WdfRequestGetFileObject(Request));
    

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Type == WdfRequestTypeDeviceControl && params.Parameters.DeviceIoControl.IoControlCode == IOCTL_MAP_BAR)
    {
        ULONG barIndex = 0;

        switch (ptFile_Context->file_type)
        {
        case FILE_TYPE_USER:
            barIndex = 0;
            break;
        case FILE_TYPE_CONTROL:
            barIndex = 1;
            break;
        case FILE_TYPE_BYPASS_0:
            barIndex = 2;
            break;
        case FILE_TYPE_BYPASS_1:
            barIndex = 3;
            break;
        case FILE_TYPE_BYPASS_2:
            barIndex = 4;
            break;
        case FILE_TYPE_BYPASS_3:
            barIndex = 5;
            break;
        default:
            TraceError(DBG_INIT, "%!FUNC! failed: file_type = %d", (LONG)ptFile_Context->file_type);
            break;
        }

        //直接在此处处理用户态的 irp 请求
        status = IoCtrlMapBarsToUser(Device, Request, barIndex);        //必须在此处映射，因为当前还处于用户态的进程上下文中

        WdfRequestCompleteWithInformation(Request, status, sizeof(XDMA_BAR_INFO));
        
        return;
    }

    status = WdfDeviceEnqueueRequest(Device, Request);
    if (!NT_SUCCESS(status))
    {
        TraceError(DBG_INIT, "%!FUNC! failed: %!STATUS!", status);
    }


    TraceInfo(DBG_INIT, "%!FUNC! is end.");
}


VOID EvtDeviceFileCreate(_In_ WDFDEVICE Device, _In_ WDFREQUEST Request, _In_ WDFFILEOBJECT FileObject)
{
    UNREFERENCED_PARAMETER(Device);

    TraceVerbose(DBG_INIT, "%!FUNC! is enter.");

    NTSTATUS status = STATUS_SUCCESS;

    PUNICODE_STRING file_name = WdfFileObjectGetFileName(FileObject);
    PFILE_CONTEXT ptFile_Context = GetFileContext(FileObject);
    if (!ptFile_Context)
    {
        TraceError(DBG_INIT, "%!FUNC! failed: ptFile_Context = NULL");
        return;
    }

    GetFileType(file_name, ptFile_Context);

    switch (ptFile_Context->file_type)
    {
    case FILE_TYPE_NONE:        //不处理
        status = STATUS_INVALID_PARAMETER;
        break;
    case FILE_TYPE_USER:
    case FILE_TYPE_CONTROL:
    case FILE_TYPE_BYPASS_0:
    case FILE_TYPE_BYPASS_1:
    case FILE_TYPE_BYPASS_2:
    case FILE_TYPE_BYPASS_3:
    case FILE_TYPE_H2C:
    case FILE_TYPE_C2H:
        break;
    default:
        status = STATUS_INVALID_PARAMETER;
        break;
    }

    WdfRequestComplete(Request, status);
    
    TraceVerbose(DBG_INIT, "%!FUNC! is end.");
}

VOID EvtFileClose(_In_ WDFFILEOBJECT FileObject)
{
    PFILE_CONTEXT ptFile_Context = GetFileContext(FileObject);
    
    //防止内存越界
    C_ASSERT(sizeof(ptFile_Context->bar_infos) / sizeof(ptFile_Context->bar_infos[0]) == sizeof(ptFile_Context->mdls) / sizeof(ptFile_Context->mdls[0]));
    
    for (size_t i = 0; i < sizeof(ptFile_Context->bar_infos) / sizeof(ptFile_Context->bar_infos[0]); i++)
    {
        if (ptFile_Context->bar_infos[i].bar_user_virtual_address && ptFile_Context->mdls[i])
        {
            MmUnmapLockedPages(ptFile_Context->bar_infos[i].bar_user_virtual_address, ptFile_Context->mdls[i]);
            IoFreeMdl(ptFile_Context->mdls[i]);

            ptFile_Context->bar_infos[i].bar_user_virtual_address = NULL;
            ptFile_Context->bar_infos[i].bar_length = 0;
            ptFile_Context->mdls[i] = NULL;
        }

    }

    UNREFERENCED_PARAMETER(FileObject);
    return;
}

VOID EvtFileCleanup(_In_ WDFFILEOBJECT FileObject)
{
    UNREFERENCED_PARAMETER(FileObject);
    return;
}
