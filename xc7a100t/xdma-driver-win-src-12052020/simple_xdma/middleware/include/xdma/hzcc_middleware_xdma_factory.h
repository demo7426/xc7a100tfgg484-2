/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	hzcc_middleware_xdma_factory.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.23
描  述: 实现xdma ip核工厂
备  注:
修改记录:

  1.  日期: 2026.08.23
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#pragma once

#include "hzcc_middleware_xdma.h"
#include "singleton.h"

namespace hzcc 
{
	namespace middleware
	{
		enum XDMA_TYPE
		{
			NONE = 0,
			XILINX,			//xilinx xdma ip类型

		};

		class LCB_DLLEXPORT CXDMA_Base_Factory : public CSingleton<CXDMA_Base_Factory>
		{
			friend class CSingleton<CXDMA_Base_Factory>;
		public:
			~CXDMA_Base_Factory() = default;

			CXDMA_Base* GetPtr(XDMA_TYPE type);

		private:

		};
	}
	
}

