/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	middleware_xdma_xilinx.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.23
描  述: 实现xilinx官方的xdma ip核的读写数据性能测试
备  注:
修改记录:

  1.  日期: 2026.08.23
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/


#include <Windows.h>
#include <SetupAPI.h>

#include "middleware_xdma_xilinx.h"
#include "debug.h"
#include "simple_xdma_public.h"

namespace hzcc
{
    namespace middleware
    {
        #define MAX_BUF_SIZE (8 * 1024 * 1024)      //单次不能超过 8MB，超出驱动自动分片
        #define ALIGNED_SIZE 256                    //内存对其大小

        int CXDMA_Xilinx::Init()
        {
            auto unDevNum = CXDMA_Base::Init();
           
            m_vecC2H_Path.clear();
            m_vecH2C_Path.clear();

            if (unDevNum >= 1)
            {
                m_vecC2H_Path.push_back(m_vecBasePath.back() + XDMA_FILE_H2C_0);
                m_vecH2C_Path.push_back(m_vecBasePath.back() + XDMA_FILE_C2H_0);
            }

#if 0
            for (int i = 0; i < m_vecBasePath.size(); i++)
            {
                switch (i)
                {
                case 0:
                    m_vecC2H_Path.push_back(m_vecBasePath[i] + XDMA_FILE_H2C_0);
                    m_vecH2C_Path.push_back(m_vecBasePath[i] + XDMA_FILE_C2H_0);
                    break;
                case 1:
                    m_vecC2H_Path.push_back(m_vecBasePath[i] + XDMA_FILE_H2C_1);
                    m_vecH2C_Path.push_back(m_vecBasePath[i] + XDMA_FILE_C2H_1);
                    break;
                case 2:
                    m_vecC2H_Path.push_back(m_vecBasePath[i] + XDMA_FILE_H2C_2);
                    m_vecH2C_Path.push_back(m_vecBasePath[i] + XDMA_FILE_C2H_2);
                    break;
                case 3:
                    m_vecC2H_Path.push_back(m_vecBasePath[i] + XDMA_FILE_H2C_3);
                    m_vecH2C_Path.push_back(m_vecBasePath[i] + XDMA_FILE_C2H_3);
                    break;
                default:
                    break;
                }

            }
#endif

            return unDevNum;
        }

        int CXDMA_Xilinx::Exit()
        {
            if (!m_vecC2H_Path.empty())
                m_vecC2H_Path.clear();

            if (!m_vecH2C_Path.empty())
                m_vecH2C_Path.clear();

            return 0;
        }


        int CXDMA_Xilinx::StartH2CTest()
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


        int CXDMA_Xilinx::StartC2HTest()
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
                hFile = CreateFile(m_vecC2H_Path[i].c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

        int CXDMA_Xilinx::StartC2H_AsyncTest()
        {
            HANDLE hFile = NULL;

            PCHAR pchReadBuf = (PCHAR)_aligned_malloc(MAX_BUF_SIZE, ALIGNED_SIZE);
            if (!pchReadBuf)
            {
                DEBUG(DEBUG_LEVEL_ERROR, "_aligned_malloc is failed.");
                return -2;
            }

            LPOVERLAPPED ptOverlapped = NULL;

            for (size_t i = 0; i < m_vecC2H_Path.size(); i++)
            {
                //测试每一个xdma设备
                hFile = CreateFile(
                    m_vecC2H_Path[i].c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED, //开启重叠IO
                    NULL
                );
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    DEBUG(DEBUG_LEVEL_ERROR, "CreateFile is failed, c2h_path = %ws.", m_vecC2H_Path[i].c_str());
                    break;
                }

                for (DWORD len = 256; len <= MAX_BUF_SIZE; len *= 2)
                {
                    ptOverlapped = static_cast<LPOVERLAPPED>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof OVERLAPPED));
                    if (!ptOverlapped)
                    {
                        DEBUG(DEBUG_LEVEL_ERROR, "HeapAlloc failed.");
                        continue;
                    }

                    if (!ReadFileEx(hFile, (LPVOID)pchReadBuf, len, ptOverlapped, CXDMA_Xilinx::ReadFile_Overlapped_Completion_Routine))
                    {
                        DEBUG(DEBUG_LEVEL_ERROR, "ReadFileEx is failed, len = %u.", len);
                        break;
                    }

                    SleepEx(INFINITE, TRUE);    //主动进入可告警等待，给操作系统机会运行你的 IO 完成函数
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

        int CXDMA_Xilinx::LoopTest()
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

                for (DWORD len = ALIGNED_SIZE; len <= MAX_BUF_SIZE; len *= 2)
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

        int CXDMA_Xilinx::StartH2C_SpeedTest(int _DevIndex)
        {
            auto func = [this](const int IsRunIndex, std::basic_string<TCHAR> _H2C_Path) {
                HANDLE hFile = NULL;

                PCHAR pchWriteBuf = (PCHAR)_aligned_malloc(MAX_BUF_SIZE, ALIGNED_SIZE);
                if (!pchWriteBuf)
                {
                    DEBUG(DEBUG_LEVEL_ERROR, "_aligned_malloc is failed.");
                    return;
                }

                DWORD lpNumberOfBytesWritten = 0;
                UINT64 lWriteSum = 0;       //写入数据的总长度;单位:Byte
                constexpr static DWORD len = MAX_BUF_SIZE;

                auto start_clock = std::chrono::steady_clock::now();
                auto end_clock = start_clock;
                auto ini_clock = start_clock;           //初始时刻
                UINT64 totalBytes = 0;                  //总数据量;单位:Byte

                //测试每一个xdma设备
                hFile = CreateFile(_H2C_Path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    DEBUG(DEBUG_LEVEL_ERROR, "CreateFile is failed, h2c_path = %ws.", _H2C_Path.c_str());
                    return;
                }

                RtlFillMemory(pchWriteBuf, MAX_BUF_SIZE, 1);

                start_clock = std::chrono::steady_clock::now();
                ini_clock = start_clock;

                while (this->m_vecH2CIsRun[IsRunIndex])
                {
                    lpNumberOfBytesWritten = 0;

                    if (!WriteFile(hFile, (LPVOID)pchWriteBuf, len, &lpNumberOfBytesWritten, NULL) && lpNumberOfBytesWritten != len)
                    {
                        DEBUG(DEBUG_LEVEL_INFO, "WriteFile is failed, len = %u, lpNumberOfBytesWritten = %u.", len, lpNumberOfBytesWritten);
                        break;
                    }

                    lWriteSum += lpNumberOfBytesWritten;
                    totalBytes += lpNumberOfBytesWritten;

                    end_clock = std::chrono::steady_clock::now();

                    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_clock - start_clock).count();

                    if (ns >= 1000'000'000)
                    {
                        {
                            SPEED_INFO tSpeedInfo = { 0 };

                            tSpeedInfo.Speed = lWriteSum * 1.0 / 1000 / 1000 / (ns * 1.0 / 1000'000'000);
                            tSpeedInfo.Time = std::chrono::duration_cast<std::chrono::milliseconds>(end_clock - ini_clock).count();
                            tSpeedInfo.AverageSpeed = (totalBytes * 1.0 / 1000 / 1000) /
                                (std::chrono::duration_cast<std::chrono::nanoseconds>(end_clock - ini_clock).count() / 1000'000'000);

                            std::unique_lock<std::mutex> lock(m_mutH2CSpeed);

                            m_vecH2CSpeed.push_back(std::move(tSpeedInfo));
                        }

                        lWriteSum = 0;

                        start_clock = std::chrono::steady_clock::now();
                        end_clock = start_clock;
                    }

                }

                CloseHandle(hFile);
                hFile = NULL;

                if (pchWriteBuf)
                {
                    _aligned_free(pchWriteBuf);
                    pchWriteBuf = NULL;
                }

                return;
                };

            this->StopH2C_SpeedTest();

            m_vecH2CSpeed.reserve(64);

            if (_DevIndex >= m_vecH2C_Path.size())
            {
                DEBUG(DEBUG_LEVEL_ERROR, "_DevIndex over of range, _DevIndex = %d.", _DevIndex);
                return -2;
            }

            //测试当前的PCI/PCIe设备
            m_vecH2CIsRun.push_back(true);
            m_vecThH2C.push_back(new std::thread(func, _DevIndex, m_vecH2C_Path[_DevIndex]));

            return 0;
        }

        std::optional<std::vector<SPEED_INFO>> CXDMA_Xilinx::GetH2C_SpeedInfo()
        {
            {
                std::unique_lock<std::mutex> lock(m_mutH2CSpeed);

                if (!m_vecH2CSpeed.empty())
                {
                    auto opt_rtn = m_vecH2CSpeed;
                    m_vecH2CSpeed.clear();

                    return opt_rtn;
                }
            }

            return std::nullopt;
        }

        int CXDMA_Xilinx::StopH2C_SpeedTest()
        {
            for (size_t i = 0; i < m_vecThH2C.size(); i++)
            {
                if (m_vecThH2C[i])
                {
                    m_vecH2CIsRun[i] = false;

                    if (m_vecThH2C[i]->joinable())
                        m_vecThH2C[i]->join();

                    delete m_vecThH2C[i];
                    m_vecThH2C[i] = nullptr;
                }
            }

            if (!m_vecH2CIsRun.empty())
                m_vecH2CIsRun.clear();

            if (!m_vecThH2C.empty())
                m_vecThH2C.clear();

            return 0;
        }

        int CXDMA_Xilinx::StartC2H_SpeedTest(int _DevIndex)
        {
            auto func = [this](const int IsRunIndex, std::basic_string<TCHAR> _C2H_Path) {
                HANDLE hFile = NULL;

                PCHAR pchReadBuf = (PCHAR)_aligned_malloc(MAX_BUF_SIZE, ALIGNED_SIZE);
                if (!pchReadBuf)
                {
                    DEBUG(DEBUG_LEVEL_ERROR, "_aligned_malloc is failed.");
                    return;
                }

                DWORD lpNumberOfBytesRead = 0;
                UINT64 lWriteSum = 0;       //写入数据的总长度;单位:Byte
                constexpr static DWORD len = MAX_BUF_SIZE;

                auto start_clock = std::chrono::steady_clock::now();
                auto end_clock = start_clock;
                auto ini_clock = start_clock;           //初始时刻
                UINT64 totalBytes = 0;                  //总数据量;单位:Byte

                //测试每一个xdma设备
                hFile = CreateFile(_C2H_Path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    DEBUG(DEBUG_LEVEL_ERROR, "CreateFile is failed, c2h_path = %ws.", _C2H_Path.c_str());
                    return;
                }

                start_clock = std::chrono::steady_clock::now();
                ini_clock = start_clock;

                while (this->m_vecC2HIsRun[IsRunIndex])
                {
                    lpNumberOfBytesRead = 0;

                    if (!ReadFile(hFile, (LPVOID)pchReadBuf, len, &lpNumberOfBytesRead, NULL) && lpNumberOfBytesRead != len)
                    {
                        DEBUG(DEBUG_LEVEL_INFO, "ReadFile is failed, len = %u, lpNumberOfBytesRead = %u.", len, lpNumberOfBytesRead);
                        break;
                    }

                    lWriteSum += lpNumberOfBytesRead;
                    totalBytes += lpNumberOfBytesRead;

                    end_clock = std::chrono::steady_clock::now();

                    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_clock - start_clock).count();

                    if (ns >= 1000'000'000)
                    {
                        {
                            SPEED_INFO tSpeedInfo = { 0 };

                            tSpeedInfo.Speed = lWriteSum * 1.0 / 1000 / 1000 / (ns * 1.0 / 1000'000'000);
                            tSpeedInfo.Time = std::chrono::duration_cast<std::chrono::milliseconds>(end_clock - ini_clock).count();
                            tSpeedInfo.AverageSpeed = (totalBytes * 1.0 / 1000 / 1000) /
                                (std::chrono::duration_cast<std::chrono::nanoseconds>(end_clock - ini_clock).count() / 1000'000'000);

                            std::unique_lock<std::mutex> lock(m_mutC2HSpeed);

                            m_vecC2HSpeed.push_back(std::move(tSpeedInfo));
                        }

                        lWriteSum = 0;

                        start_clock = std::chrono::steady_clock::now();
                        end_clock = start_clock;
                    }

                }

                CloseHandle(hFile);
                hFile = NULL;

                if (pchReadBuf)
                {
                    _aligned_free(pchReadBuf);
                    pchReadBuf = NULL;
                }

                return;
                };

            this->StopC2H_SpeedTest();

            m_vecC2HSpeed.reserve(64);

            if (_DevIndex >= m_vecC2H_Path.size())
            {
                DEBUG(DEBUG_LEVEL_ERROR, "_DevIndex over of range, _DevIndex = %d.", _DevIndex);
                return -2;
            }

            //测试当前的PCI/PCIe设备
            m_vecC2HIsRun.push_back(true);
            m_vecThC2H.push_back(new std::thread(func, _DevIndex, m_vecC2H_Path[_DevIndex]));

            return 0;
        }

        std::optional<std::vector<SPEED_INFO>> CXDMA_Xilinx::GetC2H_SpeedInfo()
        {
            {
                std::unique_lock<std::mutex> lock(m_mutC2HSpeed);

                if (!m_vecC2HSpeed.empty())
                {
                    auto opt_rtn = m_vecC2HSpeed;
                    m_vecC2HSpeed.clear();

                    return opt_rtn;
                }
            }

            return std::nullopt;
        }

        int CXDMA_Xilinx::StopC2H_SpeedTest()
        {
            for (size_t i = 0; i < m_vecThC2H.size(); i++)
            {
                if (m_vecThC2H[i])
                {
                    m_vecC2HIsRun[i] = false;

                    if (m_vecThC2H[i]->joinable())
                        m_vecThC2H[i]->join();

                    delete m_vecThC2H[i];
                    m_vecThC2H[i] = nullptr;
                }
            }

            if (!m_vecC2HIsRun.empty())
                m_vecC2HIsRun.clear();

            if (!m_vecThC2H.empty())
                m_vecThC2H.clear();

            return 0;
        }

        int CXDMA_Xilinx::StartH2C_AsyncTest()
        {
            HANDLE hFile = NULL;

            PCHAR pchWriteBuf = (PCHAR)_aligned_malloc(MAX_BUF_SIZE, ALIGNED_SIZE);
            if (!pchWriteBuf)
            {
                DEBUG(DEBUG_LEVEL_ERROR, "_aligned_malloc is failed.");
                return -2;
            }

            LPOVERLAPPED ptOverlapped = NULL;

            RtlFillMemory(pchWriteBuf, MAX_BUF_SIZE, 1);

            for (size_t i = 0; i < m_vecH2C_Path.size(); i++)
            {
                //测试每一个xdma设备
                hFile = CreateFile(
                    m_vecH2C_Path[i].c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED, //开启重叠IO
                    NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    DEBUG(DEBUG_LEVEL_ERROR, "CreateFile is failed, h2c_path = %ws.", m_vecH2C_Path[i].c_str());
                    break;
                }

                for (DWORD len = 256; len <= MAX_BUF_SIZE; len *= 2)
                {
                    ptOverlapped = static_cast<LPOVERLAPPED>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof OVERLAPPED));
                    if (!ptOverlapped)
                    {
                        DEBUG(DEBUG_LEVEL_ERROR, "HeapAlloc failed.");
                        continue;
                    }

                    if (!WriteFileEx(hFile, (LPVOID)pchWriteBuf, len, ptOverlapped, CXDMA_Xilinx::WriteFile_Overlapped_Completion_Routine))
                    {
                        DEBUG(DEBUG_LEVEL_ERROR, "WriteFileEx is failed, len = %u.", len);
                        break;
                    }

                    SleepEx(INFINITE, TRUE);    //主动进入可告警等待，给操作系统机会运行你的 IO 完成函数
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

        VOID WINAPI CXDMA_Xilinx::WriteFile_Overlapped_Completion_Routine(
            _In_    DWORD dwErrorCode,
            _In_    DWORD dwNumberOfBytesTransfered,
            _Inout_ LPOVERLAPPED lpOverlapped)
        {
            DEBUG(DEBUG_LEVEL_INFO, "WriteFileEx is success, dwNumberOfBytesTransfered = %u.", dwNumberOfBytesTransfered);

            if (lpOverlapped)
            {
                HeapFree(GetProcessHeap(), 0, lpOverlapped);
                lpOverlapped = NULL;
            }
        }

        VOID WINAPI CXDMA_Xilinx::ReadFile_Overlapped_Completion_Routine(
            _In_    DWORD dwErrorCode,
            _In_    DWORD dwNumberOfBytesTransfered,
            _Inout_ LPOVERLAPPED lpOverlapped)
        {
            DEBUG(DEBUG_LEVEL_INFO, "ReadFileEx is success, dwNumberOfBytesTransfered = %u.", dwNumberOfBytesTransfered);

            if (lpOverlapped)
            {
                HeapFree(GetProcessHeap(), 0, lpOverlapped);
                lpOverlapped = NULL;
            }
        }

    }
    

}
