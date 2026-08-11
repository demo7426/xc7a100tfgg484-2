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

namespace hzcc
{
    #define MAX_BUF_SIZE (8 * 1024 * 1024)      //单次不能超过 8MB，超出驱动自动分片
    #define ALIGNED_SIZE 256                    //内存对其大小

    // 74c7e4a9-6d5d-4a70-bc0d-20691dff9e9d
    DEFINE_GUID(GUID_DEVINTERFACE_XDMA,
        0x74c7e4a9, 0x6d5d, 0x4a70, 0xbc, 0x0d, 0x20, 0x69, 0x1d, 0xff, 0x9e, 0x9d);


    CXDMA_Test_Base::~CXDMA_Test_Base()
    {

    }


    unsigned int CXDMA_Test_Base::FindDevice(GUID tGuid)
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
            m_vecC2H_Path.push_back(m_vecBasePath.back() + L"\\c2h_" + std::to_wstring(m_vecC2H_Path.size()));
            m_vecH2C_Path.push_back(m_vecBasePath.back() + L"\\h2c_" + std::to_wstring(m_vecH2C_Path.size()));

            HeapFree(GetProcessHeap(), 0, ptSP_DEVICE_INTERFACE_DETAIL_DATA);
            ptSP_DEVICE_INTERFACE_DETAIL_DATA = NULL;
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
        hDevInfo = NULL;

        return dwIndex;
    }

    CXilinx_XDma_Test::CXilinx_XDma_Test()
    {
        auto dwDevNum = this->FindDevice(GUID_DEVINTERFACE_XDMA);

        DEBUG(DEBUG_LEVEL_INFO, "Found %u XDma device.", dwDevNum);

        for (size_t i = 0; i < m_vecBasePath.size(); i++)
        {
            DEBUG(DEBUG_LEVEL_INFO, "i = %llu, DevicePath = %ws.", i, m_vecBasePath[i].c_str());
        }

    }


    int CXilinx_XDma_Test::StartH2CTest()
    {
        HANDLE hFile = NULL;

        PCHAR pchWriteBuf = (PCHAR)_aligned_malloc(MAX_BUF_SIZE, ALIGNED_SIZE);
        if (!pchWriteBuf)
        {
            DEBUG(DEBUG_LEVEL_ERROR, "_aligned_malloc is failed.");
            return -2;
        }

        DWORD lpNumberOfBytesWritten = 0;

        for (size_t i = 0; i < m_vecH2C_Path.size(); i++)
        {
            //测试每一个xdma设备
            hFile = CreateFile(m_vecH2C_Path[i].c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                DEBUG(DEBUG_LEVEL_ERROR, "CreateFile is failed, h2c_path = %ws.", m_vecH2C_Path[i].c_str());
                break;
            }

            for (DWORD len = 256; len <= MAX_BUF_SIZE; len *= 2)
            {
                lpNumberOfBytesWritten = 0;

                if (!WriteFile(hFile, (LPVOID)pchWriteBuf, len, &lpNumberOfBytesWritten, NULL) && lpNumberOfBytesWritten != len)
                {
                    DEBUG(DEBUG_LEVEL_INFO, "WriteFile is failed, len = %u, lpNumberOfBytesWritten = %u.", len, lpNumberOfBytesWritten);
                    break;
                }

                DEBUG(DEBUG_LEVEL_INFO, "WriteFile is success, len = %u, lpNumberOfBytesWritten = %u.", len, lpNumberOfBytesWritten);
            }

            CloseHandle(hFile);
            hFile = NULL;
        }

        if (pchWriteBuf)
        {
            _aligned_free(pchWriteBuf);
            pchWriteBuf = NULL;
        }

        return 0;
    }


    int CXilinx_XDma_Test::StartC2HTest()
    {
        HANDLE hFile = NULL;

        PCHAR pchReadBuf = (PCHAR)_aligned_malloc(MAX_BUF_SIZE, ALIGNED_SIZE);
        if (!pchReadBuf)
        {
            DEBUG(DEBUG_LEVEL_ERROR, "_aligned_malloc is failed.");
            return -2;
        }

        DWORD dwNumberOfBytesRead = 0;

        for (size_t i = 0; i < m_vecC2H_Path.size(); i++)
        {
            //测试每一个xdma设备
            hFile =  CreateFile(m_vecC2H_Path[i].c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                DEBUG(DEBUG_LEVEL_ERROR, "CreateFile is failed, c2h_path = %ws.", m_vecC2H_Path[i].c_str());
                break;
            }

            for (DWORD len = 256; len <= MAX_BUF_SIZE; len *= 2)
            {
                dwNumberOfBytesRead = 0;

                if (!ReadFile(hFile, (LPVOID)pchReadBuf, len, &dwNumberOfBytesRead, NULL) && dwNumberOfBytesRead != len)
                {
                    DEBUG(DEBUG_LEVEL_INFO, "ReadFile is failed, len = %u, dwNumberOfBytesRead = %u.", len, dwNumberOfBytesRead);
                    break;
                }

                DEBUG(DEBUG_LEVEL_INFO, "ReadFile is success, len = %u, dwNumberOfBytesRead = %u.", len, dwNumberOfBytesRead);
            }

            CloseHandle(hFile);
            hFile = NULL;
        }

        if (pchReadBuf)
        {
            _aligned_free(pchReadBuf);
            pchReadBuf = NULL;
        }

        return 0;
    }

    int CXilinx_XDma_Test::LoopTest()
    {
        HANDLE hC2H_File = NULL;
        HANDLE hH2C_File = NULL;

        PCHAR pchReadBuf = (PCHAR)_aligned_malloc(MAX_BUF_SIZE, ALIGNED_SIZE);
        if (!pchReadBuf)
        {
            DEBUG(DEBUG_LEVEL_ERROR, "_aligned_malloc is failed.");
            return -2;
        }
        
        PCHAR pchWriteBuf = (PCHAR)_aligned_malloc(MAX_BUF_SIZE, ALIGNED_SIZE);
        if (!pchWriteBuf)
        {
            DEBUG(DEBUG_LEVEL_ERROR, "_aligned_malloc is failed.");
            return -2;
        }

        DWORD lpNumberOfBytesWritten = 0;
        DWORD dwNumberOfBytesRead = 0;

        for (size_t i = 0; i < MAX_BUF_SIZE; i++)
        {
            pchWriteBuf[i] = i % UCHAR_MAX;
        }

        for (size_t i = 0; i < m_vecC2H_Path.size(); i++)
        {
            //测试每一个xdma设备
            hC2H_File = CreateFile(m_vecC2H_Path[i].c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hC2H_File == INVALID_HANDLE_VALUE)
            {
                DEBUG(DEBUG_LEVEL_ERROR, "CreateFile is failed, c2h_path = %ws.", m_vecC2H_Path[i].c_str());
                break;
            }

            hH2C_File = CreateFile(m_vecH2C_Path[i].c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hH2C_File == INVALID_HANDLE_VALUE)
            {
                DEBUG(DEBUG_LEVEL_ERROR, "CreateFile is failed, h2c_path = %ws.", m_vecH2C_Path[i].c_str());
                break;
            }

            for (DWORD len = ALIGNED_SIZE; len < MAX_BUF_SIZE; len *= 2)
            {
                lpNumberOfBytesWritten = 0;
                if (!WriteFile(hH2C_File, (LPVOID)pchWriteBuf, len, &lpNumberOfBytesWritten, NULL) && lpNumberOfBytesWritten != len)
                {
                    DEBUG(DEBUG_LEVEL_ERROR, "WriteFile is failed, len = %u, lpNumberOfBytesWritten = %u.", len, lpNumberOfBytesWritten);
                }

                dwNumberOfBytesRead = 0;
                if (!ReadFile(hC2H_File, (LPVOID)pchReadBuf, len, &dwNumberOfBytesRead, NULL) && dwNumberOfBytesRead != len)
                {
                    DEBUG(DEBUG_LEVEL_ERROR, "ReadFile is failed, len = %u, dwNumberOfBytesRead = %u.", len, dwNumberOfBytesRead);
                }

                //内存比较
                if (memcmp(pchWriteBuf, pchReadBuf, len))
                    DEBUG(DEBUG_LEVEL_INFO, "Memory data inconsistency, len = %u.", len);
                else
                    DEBUG(DEBUG_LEVEL_INFO, "Memory data consistency, len = %u.", len);
            }


            CloseHandle(hH2C_File);
            hH2C_File = NULL;

            CloseHandle(hC2H_File);
            hC2H_File = NULL;
        }

        if (pchWriteBuf)
        {
            _aligned_free(pchWriteBuf);
            pchWriteBuf = NULL;
        }
        
        if (pchReadBuf)
        {
            _aligned_free(pchReadBuf);
            pchReadBuf = NULL;
        }

        return 0;
    }


    

}

