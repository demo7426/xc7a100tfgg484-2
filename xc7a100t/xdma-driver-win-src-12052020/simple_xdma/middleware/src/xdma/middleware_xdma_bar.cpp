/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	middleware_xdma_bar.cpp
作  者:	钱锐      版本: V1.0     新建日期: 2026.09.04
描  述: 实现xilinx官方的bypass，及将bar物理地址直接映射到用户态
备  注:
修改记录:

  1.  日期: 2026.09.04
	  作者: 钱锐
	  内容:
		  1) 此为模板第一个版本；
	  版本:V1.0

*************************************************/

#pragma once

#include "middleware_xdma_bar.h"
#include "simple_xdma_public.h"
#include "debug.h"

namespace hzcc
{
	namespace middleware
	{
		int CXDMA_Bar::Init()
		{
            auto unDevNum = CXDMA_Base::Init();

			m_vecUser_Path.clear();
			m_vecControl_Path.clear();
			m_vecBypass_0_Path.clear();
			m_vecBypass_1_Path.clear();
			m_vecBypass_2_Path.clear();
			m_vecBypass_3_Path.clear();

			for (int i = 0; i < m_vecBasePath.size(); i++)
			{
				m_vecUser_Path.push_back(m_vecBasePath[i] + XDMA_FILE_USER);
				m_vecControl_Path.push_back(m_vecBasePath[i] + XDMA_FILE_CONTROL);
				m_vecBypass_0_Path.push_back(m_vecBasePath[i] + XDMA_FILE_BYPASS_0);
				m_vecBypass_1_Path.push_back(m_vecBasePath[i] + XDMA_FILE_BYPASS_1);
				m_vecBypass_2_Path.push_back(m_vecBasePath[i] + XDMA_FILE_BYPASS_2);
				m_vecBypass_3_Path.push_back(m_vecBasePath[i] + XDMA_FILE_BYPASS_3);
			}

            return unDevNum;
		}

		int CXDMA_Bar::Exit()
		{
			if (!m_vecUser_Path.empty())
				m_vecUser_Path.clear();

			if (!m_vecControl_Path.empty())
				m_vecControl_Path.clear();

			if (!m_vecBypass_0_Path.empty())
				m_vecBypass_0_Path.clear();
			
			if (!m_vecBypass_1_Path.empty())
				m_vecBypass_1_Path.clear();
			
			if (!m_vecBypass_2_Path.empty())
				m_vecBypass_2_Path.clear();
			
			if (!m_vecBypass_3_Path.empty())
				m_vecBypass_3_Path.clear();

			return 0;
		}

		int CXDMA_Bar::BarReadWrite_Test()
		{
			auto func = [](std::vector<std::basic_string<TCHAR>> path) {

				HANDLE hFile = NULL;

				DWORD lpNumberOfBytesWritten = 0;
				DWORD dwRet = 0;

				for (size_t i = 0; i < path.size(); i++)
				{
					XDMA_BAR_INFO tXDMA_Bar_Info = { 0 };

					//测试每一个xdma设备
					hFile = CreateFile(path[i].c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFile == INVALID_HANDLE_VALUE)
					{
						DEBUG(DEBUG_LEVEL_ERROR, "CreateFile is failed, path = %ws.", path[i].c_str());
						break;
					}

					if (!DeviceIoControl(hFile, IOCTL_MAP_BAR, NULL, 0, &tXDMA_Bar_Info, sizeof(tXDMA_Bar_Info), &dwRet, 0))
					{
						DEBUG(DEBUG_LEVEL_ERROR, "DeviceIoControl is failed, path = %ws.", path[i].c_str());
						break;
					}

					//for (size_t j = 0; j < tXDMA_Bar_Info.bar_length; j++)
					for (size_t j = 0; j < 64; j++)
					{
						printf("%02x ", *(((UCHAR*)tXDMA_Bar_Info.bar_user_virtual_address) + j));

						if (j % 16 == 15)
							printf("\n");
					}

					CloseHandle(hFile);
					hFile = NULL;
				}
				};

			func(m_vecUser_Path);
			DEBUG(DEBUG_LEVEL_INFO, "");

			func(m_vecControl_Path);
			DEBUG(DEBUG_LEVEL_INFO, "");

			func(m_vecBypass_0_Path);
			DEBUG(DEBUG_LEVEL_INFO, "");

			func(m_vecBypass_1_Path);
			DEBUG(DEBUG_LEVEL_INFO, "");

			func(m_vecBypass_2_Path);
			DEBUG(DEBUG_LEVEL_INFO, "");

			func(m_vecBypass_3_Path);
			DEBUG(DEBUG_LEVEL_INFO, "");

            return 0;
		}
	}
	

}