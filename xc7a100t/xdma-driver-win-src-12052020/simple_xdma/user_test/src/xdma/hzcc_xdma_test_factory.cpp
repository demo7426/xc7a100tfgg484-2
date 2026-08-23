/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	hzcc_xdma_test_factory.cpp
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

#include "hzcc_xdma_test_factory.h"
#include "hzcc_xilinx_xdma_test.h"

namespace hzcc
{
    CXDMA_Test_Base* hzcc::CXDMA_Test_Base_Factory::GetPtr(XDMA_TYPE type)
    {
        CXDMA_Test_Base* pCXDMA_Test_Base = nullptr;

        switch (type)
        {
        case hzcc::NONE:
            break;
        case hzcc::XILINX:
            pCXDMA_Test_Base = new CXilinx_XDMA_Test;
            break;
        default:
            break;
        }

        return pCXDMA_Test_Base;
    }
}


