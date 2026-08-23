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

#include "debug.h"
#include "hzcc_xdma_test.h"
#include "hzcc_xdma_test_factory.h"

enum class _USER_CMD_TYPE: int32_t
{
	NONE = -1,				//无效参数
	C2H_TEST = 0,			//同步c2h测试
	C2H_ASYNC_TEST,			//异步c2h测试
	H2C_TEST,				//同步h2c测试
	H2C_ASYNC_TEST,			//异步h2c测试
	LOOP_TEST,				//回环测试
	H2C_SPEED_TEST,			//h2c速度测试
	C2H_SPEED_TEST,			//c2h速度测试
	FULL_DUPLEX_TEST,		//全双工测试
};

int main(int argc, char* argv[])
{
	try
	{
		_USER_CMD_TYPE eCmd = _USER_CMD_TYPE::H2C_SPEED_TEST;

		if(argc > 1)
			eCmd = static_cast<_USER_CMD_TYPE>(std::atoi(argv[1]));

		hzcc::CXDMA_Test_Base* pcXDMA_Test = hzcc::CXDMA_Test_Base_Factory::GetInstance()->GetPtr(hzcc::XDMA_TYPE::XILINX);
		DEBUG(DEBUG_LEVEL_INFO, "");

		switch (eCmd)
		{
		case _USER_CMD_TYPE::C2H_TEST:
			pcXDMA_Test->StartC2HTest();
			break;
		case _USER_CMD_TYPE::C2H_ASYNC_TEST:
			pcXDMA_Test->StartC2H_AsyncTest();
			break;
		case _USER_CMD_TYPE::H2C_TEST:
			pcXDMA_Test->StartH2CTest();
			break;
		case _USER_CMD_TYPE::H2C_ASYNC_TEST:
			pcXDMA_Test->StartH2C_AsyncTest();
			break;
		case _USER_CMD_TYPE::LOOP_TEST:
			pcXDMA_Test->LoopTest();
			break;
		case _USER_CMD_TYPE::H2C_SPEED_TEST:		
		{
			pcXDMA_Test->StartH2C_SpeedTest(0);

			while (1)
			{
				auto opt_rtn = pcXDMA_Test->GetH2C_SpeedInfo();

				if (opt_rtn.has_value())
				{
					auto vecSpeed = opt_rtn.value();
					for (size_t i = 0; i < vecSpeed.size(); i++)
					{
						DEBUG(DEBUG_LEVEL_INFO, "H2C speed = %.06f MB/s.", vecSpeed[i]);
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			pcXDMA_Test->StopH2C_SpeedTest();
		}
		break;
		case _USER_CMD_TYPE::C2H_SPEED_TEST:		
		{
			pcXDMA_Test->StartC2H_SpeedTest(0);

			while (1)
			{
				auto opt_rtn = pcXDMA_Test->GetC2H_SpeedInfo();

				if (opt_rtn.has_value())
				{
					auto vecSpeed = opt_rtn.value();
					for (size_t i = 0; i < vecSpeed.size(); i++)
					{
						DEBUG(DEBUG_LEVEL_INFO, "C2H speed = %.06f MB/s.", vecSpeed[i]);
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			pcXDMA_Test->StopC2H_SpeedTest();
		}
		break;
		case _USER_CMD_TYPE::FULL_DUPLEX_TEST:
		{
			pcXDMA_Test->StartH2C_SpeedTest(0);
			pcXDMA_Test->StartC2H_SpeedTest(0);

			while (1)
			{
				auto opt_rtn = pcXDMA_Test->GetH2C_SpeedInfo();

				if (opt_rtn.has_value())
				{
					auto vecSpeed = opt_rtn.value();
					for (size_t i = 0; i < vecSpeed.size(); i++)
					{
						DEBUG(DEBUG_LEVEL_INFO, "H2C speed = %.06f MB/s.", vecSpeed[i]);
					}
				}

				opt_rtn = pcXDMA_Test->GetC2H_SpeedInfo();

				if (opt_rtn.has_value())
				{
					auto vecSpeed = opt_rtn.value();
					for (size_t i = 0; i < vecSpeed.size(); i++)
					{
						DEBUG(DEBUG_LEVEL_INFO, "C2H speed = %.06f MB/s.", vecSpeed[i]);
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			pcXDMA_Test->StopC2H_SpeedTest();
			pcXDMA_Test->StopH2C_SpeedTest();
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



