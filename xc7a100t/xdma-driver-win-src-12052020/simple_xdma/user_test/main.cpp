/*************************************************
Copyright (C), 2026-2040    , Hang Zhou Chang Chuan Co., Ltd.
文件名:	main.cpp
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.10
描  述: 测试xilinx官方的xdma ip核的读写数据性能
备  注:	
修改记录:

  1.  日期: 2026.08.10
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include <iostream>

#include "Debug.h"
#include "hzcc_xdma_test.h"

int main()
{
	try
	{
		hzcc::CXDMA_Test_Base* pcXDma_Test = new hzcc::CXilinx_XDma_Test;
		DEBUG(DEBUG_LEVEL_INFO, "");

		pcXDma_Test->StartC2HTest();
		DEBUG(DEBUG_LEVEL_INFO, "");

		pcXDma_Test->StartH2CTest();
		DEBUG(DEBUG_LEVEL_INFO, "");

		pcXDma_Test->LoopTest();
	}
	catch (const std::exception& e)
	{
		DEBUG(DEBUG_LEVEL_ERROR, e.what());
	}



    return EXIT_SUCCESS;
}



