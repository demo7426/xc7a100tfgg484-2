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

#include <INITGUID.H>

#include <windows.h>
#include <vector>
#include <string>

namespace hzcc
{
	class CXDMA_Test_Base
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
		/// 开始c2h数据测试
		/// </summary>
		/// <returns></returns>
		virtual int StartC2HTest() = 0;
		
		/// <summary>
		/// 回环测试
		/// </summary>
		/// <returns></returns>
		virtual int LoopTest() = 0;

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

	class CXilinx_XDma_Test: public CXDMA_Test_Base
	{
	public:
		CXilinx_XDma_Test();
		~CXilinx_XDma_Test() {};

		/// <summary>
		/// 开始h2c数据测试
		/// </summary>
		/// <returns></returns>
		int StartH2CTest();

		/// <summary>
		/// 开始c2h数据测试
		/// </summary>
		/// <returns></returns>
		int StartC2HTest();

		/// <summary>
		/// 回环测试
		/// </summary>
		/// <returns></returns>
		int LoopTest();
	};

}
