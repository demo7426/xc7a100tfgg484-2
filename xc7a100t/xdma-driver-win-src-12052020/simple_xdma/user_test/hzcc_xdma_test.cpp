/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	hzcc_xdma_test.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.10
描  述: 实现xilinx官方的xdma ip核的读写数据性能测试
备  注:
修改记录:

  1.  日期: 2026.08.10
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include <Windows.h>
#include <SetupAPI.h>

#include "Debug.h"

#include "hzcc_xdma_test.h"

// 74c7e4a9-6d5d-4a70-bc0d-20691dff9e9d
DEFINE_GUID(GUID_DEVINTERFACE_XDMA,
    0x74c7e4a9, 0x6d5d, 0x4a70, 0xbc, 0x0d, 0x20, 0x69, 0x1d, 0xff, 0x9e, 0x9d);

namespace hzcc
{
    CXDMA_Test_Base::~CXDMA_Test_Base()
    {

    }


    CXilinx_XDma_Test::CXilinx_XDma_Test()
    {
        auto dwDevNum = this->FindDevice(GUID_DEVINTERFACE_XDMA);

        DEBUG(DEBUG_LEVEL_INFO, "Found %u XDma device.", dwDevNum);
    }

    int CXilinx_XDma_Test::StartH2CTest()
    {
        return 0;
    }

    int CXilinx_XDma_Test::StartC2HTest()
    {
        return 0;
    }

    unsigned int CXilinx_XDma_Test::FindDevice(GUID tGuid)
    {
        HDEVINFO hDevInfo = SetupDiGetClassDevs((LPGUID)&tGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE); //返回 设备信息集 的句柄，其中包含本地计算机请求的设备信息元素。
        if (hDevInfo == INVALID_HANDLE_VALUE)
        {
            DEBUG(DEBUG_LEVEL_ERROR, "SetupDiGetClassDevs is failed.");
            return -2;
        }

        SP_DEVICE_INTERFACE_DATA tSP_DEVICE_INTERFACE_DATA = { 0 };
        tSP_DEVICE_INTERFACE_DATA.cbSize = sizeof SP_DEVICE_INTERFACE_DATA;

        DWORD dwIndex = 0;

        //SetupDiEnumDeviceInterfaces 函数枚举包含在设备信息集中的设备接口
        for (DWORD dwIndex = 0; dwIndex < SetupDiEnumDeviceInterfaces(hDevInfo, NULL, (LPGUID)&tGuid, dwIndex, &tSP_DEVICE_INTERFACE_DATA); dwIndex++)
        {
            DWORD dwDeviceInterfaceDetailDataSize = 0;

            //获取当前设置详细信息需要的大小
            if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &tSP_DEVICE_INTERFACE_DATA, NULL, 0, &dwDeviceInterfaceDetailDataSize, NULL) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            {
                DEBUG(DEBUG_LEVEL_ERROR, "SetupDiGetDeviceInterfaceDetail is failed.");
                break;
            }

            //分配详细信息的堆内存
            PSP_DEVICE_INTERFACE_DETAIL_DATA ptSP_DEVICE_INTERFACE_DETAIL_DATA = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwDeviceInterfaceDetailDataSize);
            if (ptSP_DEVICE_INTERFACE_DETAIL_DATA == NULL)
            {
                DEBUG(DEBUG_LEVEL_ERROR, "HeapAlloc is failed.");
                break;
            }

            ptSP_DEVICE_INTERFACE_DETAIL_DATA->cbSize = sizeof SP_DEVICE_INTERFACE_DETAIL_DATA;

            if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &tSP_DEVICE_INTERFACE_DATA, ptSP_DEVICE_INTERFACE_DETAIL_DATA, dwDeviceInterfaceDetailDataSize, NULL, NULL) == false)
            {
                DEBUG(DEBUG_LEVEL_ERROR, "SetupDiGetDeviceInterfaceDetail is failed.");

                HeapFree(GetProcessHeap(), 0, ptSP_DEVICE_INTERFACE_DETAIL_DATA);
                ptSP_DEVICE_INTERFACE_DETAIL_DATA = NULL;

                break;
            }

            HeapFree(GetProcessHeap(), 0, ptSP_DEVICE_INTERFACE_DETAIL_DATA);
            ptSP_DEVICE_INTERFACE_DETAIL_DATA = NULL;
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
        hDevInfo = NULL;

        return dwIndex;
    }

    

}

