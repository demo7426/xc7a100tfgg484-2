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

#pragma once

#include "middleware_xdma.h"

namespace hzcc
{
	namespace middleware
	{
		class CXDMA_Xilinx : public CXDMA_Base
		{
		public:
			using CXDMA_Base::CXDMA_Base;
			~CXDMA_Xilinx() = default;

			/// <summary>
			/// 初始化
			/// </summary>
			/// <returns>PCIe卡数量</returns>
			int Init() override;

			/// <summary>
			/// 正常退出
			/// </summary>
			/// <returns></returns>
			int Exit() override;

			/// <summary>
			/// 开始h2c数据测试
			/// </summary>
			/// <returns></returns>
			int StartH2CTest() override;

			/// <summary>
			/// 开始h2c数据异步测试
			/// </summary>
			/// <returns></returns>
			int StartH2C_AsyncTest() override;

			/// <summary>
			/// 开始c2h数据测试
			/// </summary>
			/// <returns></returns>
			int StartC2HTest() override;

			/// <summary>
			/// 开始c2h数据异步测试
			/// </summary>
			/// <returns></returns>
			int StartC2H_AsyncTest() override;

			/// <summary>
			/// 回环测试
			/// </summary>
			/// <returns></returns>
			int LoopTest() override;

		public:
			/// <summary>
			/// 开始h2c速度测试
			/// </summary>
			/// <param name="_DevIndex">设备索引</param>
			/// <returns></returns>
			int StartH2C_SpeedTest(int _DevIndex) override;

			/// <summary>
			/// 获取h2c的速度信息
			/// </summary>
			/// <returns>多个std::pair<速度信息;单位:MB/s--时刻信息;单位:毫秒></returns>
			std::optional<std::vector<SPEED_INFO>> GetH2C_SpeedInfo() override;

			/// <summary>
			/// 停止h2c速度测试
			/// </summary>
			/// <returns></returns>
			int StopH2C_SpeedTest() override;

		public:
			/// <summary>
			/// 开始c2h速度测试
			/// </summary>
			/// <param name="_DevIndex">设备索引</param>
			/// <returns></returns>
			int StartC2H_SpeedTest(int _DevIndex) override;

			/// <summary>
			/// 获取c2h的速度信息
			/// </summary>
			/// <returns>多个std::pair<速度信息;单位:MB/s--时刻信息;单位:毫秒></returns>
			std::optional<std::vector<SPEED_INFO>> GetC2H_SpeedInfo() override;

			/// <summary>
			/// 停止c2h速度测试
			/// </summary>
			/// <returns></returns>
			int StopC2H_SpeedTest() override;

		private:
			/// <summary>
			/// 
			/// </summary>
			/// <param name="dwErrorCode"></param>
			/// <param name="dwNumberOfBytesTransfered"></param>
			/// <param name="lpOverlapped"></param>
			/// <returns></returns>
			static VOID WINAPI WriteFile_Overlapped_Completion_Routine(
				_In_    DWORD dwErrorCode,
				_In_    DWORD dwNumberOfBytesTransfered,
				_Inout_ LPOVERLAPPED lpOverlapped
			);

			static VOID WINAPI ReadFile_Overlapped_Completion_Routine(
				_In_    DWORD dwErrorCode,
				_In_    DWORD dwNumberOfBytesTransfered,
				_Inout_ LPOVERLAPPED lpOverlapped
			);


		private:
			std::vector<std::basic_string<TCHAR>> m_vecC2H_Path;		//c2h设备接口
			std::vector<std::basic_string<TCHAR>> m_vecH2C_Path;		//h2c设备接口

			std::vector<std::thread*> m_vecThH2C;		//h2c线程对象
			std::vector<int8_t> m_vecH2CIsRun;

			std::vector<std::thread*> m_vecThC2H;		//c2h线程对象
			std::vector<int8_t> m_vecC2HIsRun;

			std::vector<SPEED_INFO> m_vecH2CSpeed;
			std::mutex m_mutH2CSpeed;

			std::vector<SPEED_INFO> m_vecC2HSpeed;
			std::mutex m_mutC2HSpeed;
		};
	}
	

}