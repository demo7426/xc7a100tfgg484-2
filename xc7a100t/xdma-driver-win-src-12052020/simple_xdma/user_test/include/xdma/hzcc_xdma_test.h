/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	hzcc_xdma_test.h
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
	typedef struct _SPEED_INFO
	{
		double Speed;			//速度信息;单位:MB/s
		long long Time;			//时刻信息;单位:毫秒
		double AverageSpeed;	//平均速度;单位:MB/s
	}SPEED_INFO, *PSPEED_INFO;

	class LCB_DLLEXPORT CXDMA_Test_Base
	{
	public:
		CXDMA_Test_Base() = default;
		virtual ~CXDMA_Test_Base() = 0;

		/// <summary>
		/// 初始化
		/// </summary>
		/// <returns></returns>
		virtual int Init() = 0;
		
		/// <summary>
		/// 正常退出
		/// </summary>
		/// <returns></returns>
		virtual int Exit() = 0;

		/// <summary>
		/// 开始h2c数据测试
		/// </summary>
		/// <returns></returns>
		virtual int StartH2CTest();

		/// <summary>
		/// 开始h2c数据异步测试
		/// </summary>
		/// <returns></returns>
		virtual int StartH2C_AsyncTest();

		/// <summary>
		/// 开始c2h数据测试
		/// </summary>
		/// <returns></returns>
		virtual int StartC2HTest();

		/// <summary>
		/// 开始c2h数据异步测试
		/// </summary>
		/// <returns></returns>
		virtual int StartC2H_AsyncTest();
		
		/// <summary>
		/// 回环测试
		/// </summary>
		/// <returns></returns>
		virtual int LoopTest();

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
		virtual std::optional<std::vector<SPEED_INFO>> GetH2C_SpeedInfo();

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
		virtual std::optional<std::vector<SPEED_INFO>> GetC2H_SpeedInfo();

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


}
