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
#include <thread>
#include <chrono>

#include "Debug.h"
#include "hzcc_xdma_test.h"

int main(int argc, char* argv[])
{
	try
	{
		int nCmd = 4;

		if(argc > 1)
			nCmd = std::atoi(argv[1]);

		hzcc::CXDMA_Test_Base* pcXDma_Test = new hzcc::CXilinx_XDma_Test;
		DEBUG(DEBUG_LEVEL_INFO, "");

		switch (nCmd)
		{
		case 0:
			pcXDma_Test->StartC2HTest();
			break;
		case 1:
			pcXDma_Test->StartC2H_AsyncTest();
			break;
		case 2:
			pcXDma_Test->StartH2CTest();
			break;
		case 3:
			pcXDma_Test->StartH2C_AsyncTest();
			break;
		case 4:
			pcXDma_Test->LoopTest();
			break;
		case 5:
		{
			pcXDma_Test->StartH2C_SpeedTest();

			while (1)
			{
				auto opt_rtn = pcXDma_Test->GetH2C_SpeedInfo();

				if (opt_rtn.has_value())
				{
					auto vecSpeed = opt_rtn.value();
					for (size_t i = 0; i < vecSpeed.size(); i++)
					{
						DEBUG(DEBUG_LEVEL_INFO, "H2C speed = %.04f MB/s.", vecSpeed[i]);
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			pcXDma_Test->StopH2C_SpeedTest();
		}
			break;
		default:
			break;
		}

		

	}
	catch (const std::exception& e)
	{
		DEBUG(DEBUG_LEVEL_ERROR, e.what());
	}

    return EXIT_SUCCESS;
}



