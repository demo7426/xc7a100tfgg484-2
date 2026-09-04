/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	middleware_xdma.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.10
描  述: 实现xdma ip核的读写数据性能测试
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

#include "debug.h"

#include "middleware_xdma.h"
#include "simple_xdma_public.h"

namespace hzcc
{
    namespace middleware
    {
        CXDMA_Base::~CXDMA_Base()
        {

        }

        int CXDMA_Base::Init()
        {
            auto unDevNum = this->FindDevice(GUID_DEVINTERFACE_XDMA);

            DEBUG(DEBUG_LEVEL_INFO, "Found %u XDma device.", unDevNum);

            for (size_t i = 0; i < m_vecBasePath.size(); i++)
            {
                DEBUG(DEBUG_LEVEL_INFO, "i = %llu, DevicePath = %ws.", i, m_vecBasePath[i].c_str());
            }

            return unDevNum;
        }

        int CXDMA_Base::Exit()
        {
            if(!m_vecBasePath.empty())
                m_vecBasePath.clear();

            return 0;
        }


        int CXDMA_Base::StartH2CTest()
        {
            return 0;
        }


        int CXDMA_Base::StartH2C_AsyncTest()
        {
            return 0;
        }

        int CXDMA_Base::StartC2HTest()
        {
            return 0;
        }

        int CXDMA_Base::StartC2H_AsyncTest()
        {
            return 0;
        }

        int CXDMA_Base::LoopTest()
        {
            return 0;
        }

        int CXDMA_Base::StartH2C_SpeedTest(int _DevIndex)
        {
            return 0;
        }

        std::optional<std::vector<SPEED_INFO>> CXDMA_Base::GetH2C_SpeedInfo()
        {
            return std::nullopt;
        }

        int CXDMA_Base::StopH2C_SpeedTest()
        {
            return 0;
        }

        int CXDMA_Base::StartC2H_SpeedTest(int _DevIndex)
        {
            return 0;
        }

        std::optional<std::vector<SPEED_INFO>> CXDMA_Base::GetC2H_SpeedInfo()
        {
            return std::nullopt;
        }

        int CXDMA_Base::StopC2H_SpeedTest()
        {
            return 0;
        }

        int CXDMA_Base::BarReadWrite_Test()
        {
            return 0;
        }

        unsigned int CXDMA_Base::FindDevice(GUID tGuid)
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

            m_vecBasePath.clear();

            //SetupDiEnumDeviceInterfaces 函数枚举包含在设备信息集中的设备接口
            for (dwIndex = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, (LPGUID)&tGuid, dwIndex, &tSP_DEVICE_INTERFACE_DATA); dwIndex++)
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

                m_vecBasePath.push_back(std::wstring(ptSP_DEVICE_INTERFACE_DETAIL_DATA->DevicePath));

                HeapFree(GetProcessHeap(), 0, ptSP_DEVICE_INTERFACE_DETAIL_DATA);
                ptSP_DEVICE_INTERFACE_DETAIL_DATA = NULL;
            }

            SetupDiDestroyDeviceInfoList(hDevInfo);
            hDevInfo = NULL;

            return dwIndex;
        }

    }
}

