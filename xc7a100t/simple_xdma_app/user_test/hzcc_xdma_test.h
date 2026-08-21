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

#pragma once

#define LCB_DLLEXPORT __declspec(dllexport)

#include <INITGUID.H>

#include <windows.h>
#include <vector>
#include <string>
#include <thread>
#include <optional>
#include <mutex>

namespace hzcc
{
	class LCB_DLLEXPORT CXDMA_Test_Base
	{
	public:
		CXDMA_Test_Base() = default;
		virtual ~CXDMA_Test_Base() = 0;

		/// <summary>
		/// 开始h2c数据测试
		/// </summary>
		/// <returns></returns>
		virtual int StartH2CTest() = 0;

		/// <summary>
		/// 开始h2c数据异步测试
		/// </summary>
		/// <returns></returns>
		virtual int StartH2C_AsyncTest();

		/// <summary>
		/// 开始c2h数据测试
		/// </summary>
		/// <returns></returns>
		virtual int StartC2HTest() = 0;

		/// <summary>
		/// 开始c2h数据异步测试
		/// </summary>
		/// <returns></returns>
		virtual int StartC2H_AsyncTest();
		
		/// <summary>
		/// 回环测试
		/// </summary>
		/// <returns></returns>
		virtual int LoopTest() = 0;

	public:
		/// <summary>
		/// 开始h2c速度测试
		/// </summary>
		/// <param name="_DevIndex">设备索引</param>
		/// <returns></returns>
		virtual int StartH2C_SpeedTest(int _DevIndex);
		
		/// <summary>
		/// 获取h2c的速度信息
		/// </summary>
		/// <returns></returns>
		virtual std::optional<std::vector<double>> GetH2C_SpeedInfo();

		/// <summary>
		/// 停止h2c速度测试
		/// </summary>
		/// <returns></returns>
		virtual int StopH2C_SpeedTest();

	public:
		/// <summary>
		/// 开始c2h速度测试
		/// </summary>
		/// <param name="_DevIndex">设备索引</param>
		/// <returns></returns>
		virtual int StartC2H_SpeedTest(int _DevIndex);
		
		/// <summary>
		/// 获取c2h的速度信息
		/// </summary>
		/// <returns></returns>
		virtual std::optional<std::vector<double>> GetC2H_SpeedInfo();

		/// <summary>
		/// 停止c2h速度测试
		/// </summary>
		/// <returns></returns>
		virtual int StopC2H_SpeedTest();

	protected:
		std::vector<std::basic_string<TCHAR>> m_vecBasePath;		//设备接口
		std::vector<std::basic_string<TCHAR>> m_vecC2H_Path;		//c2h设备接口
		std::vector<std::basic_string<TCHAR>> m_vecH2C_Path;		//h2c设备接口

	protected:
		/// <summary>
		/// 查找匹配的设备
		/// </summary>
		/// <param name=""></param>
		/// <returns>设备数量</returns>
		unsigned int FindDevice(GUID tGuid);
	};

	class LCB_DLLEXPORT CXilinx_XDma_Test: public CXDMA_Test_Base
	{
	public:
		CXilinx_XDma_Test();
		~CXilinx_XDma_Test() {};

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
		/// <returns>速度信息;单位:MB/s</returns>
		std::optional<std::vector<double>> GetH2C_SpeedInfo() override;

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
		/// <returns></returns>
		std::optional<std::vector<double>> GetC2H_SpeedInfo() override;

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
		std::vector<std::jthread*> m_vecJThH2C;		//h2c线程对象
		std::vector<std::jthread*> m_vecJThC2H;		//c2h线程对象
		
		std::vector<double> m_vecH2CSpeed;
		std::mutex m_mutH2CSpeed;

		std::vector<double> m_vecC2HSpeed;
		std::mutex m_mutC2HSpeed;
	};

}
