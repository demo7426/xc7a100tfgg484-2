/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	middleware_xdma_bar.h
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

#include "middleware_xdma.h"

namespace hzcc
{
	namespace middleware
	{
		class CXDMA_Bar : public CXDMA_Base
		{
		public:
			using CXDMA_Base::CXDMA_Base;
			~CXDMA_Bar() = default;

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
			/// bar读写测试
			/// </summary>
			/// <returns></returns>
			int BarReadWrite_Test() override;

		private:
			std::vector<std::basic_string<TCHAR>> m_vecUser_Path;		
			std::vector<std::basic_string<TCHAR>> m_vecControl_Path;		
			std::vector<std::basic_string<TCHAR>> m_vecBypass_Path;		
		};
	}
	

}