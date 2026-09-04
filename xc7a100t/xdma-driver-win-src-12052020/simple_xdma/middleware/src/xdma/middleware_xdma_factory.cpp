/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	middleware_xdma_factory.cpp
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

#include "middleware_xdma_factory.h"
#include "middleware_xdma_xilinx.h"
#include "middleware_xdma_bar.h"

namespace hzcc
{
    namespace middleware
    {
        CXDMA_Base* hzcc::middleware::CXDMA_Base_Factory::GetPtr(XDMA_TYPE type)
        {
            CXDMA_Base* pCXDMA_Test_Base = nullptr;

            switch (type)
            {
            case hzcc::middleware::NONE:
                break;
            case hzcc::middleware::XILINX:
                pCXDMA_Test_Base = new CXDMA_Xilinx;
                break;
            case hzcc::middleware::BAR:
                pCXDMA_Test_Base = new CXDMA_Bar;
                break;
            default:
                break;
            }

            return pCXDMA_Test_Base;
        }
    }
    
}


