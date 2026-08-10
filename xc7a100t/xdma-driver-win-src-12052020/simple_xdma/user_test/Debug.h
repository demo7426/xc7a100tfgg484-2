/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	Debug.h
作  者:	钱锐      版本: V1.0.0     新建日期: 2024.08.15
描  述: 调试日志文件
备  注:	支持linux和windows操作系统
修改记录:

  1.  日期: 2024.08.15
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0.0

*************************************************/

#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <stdio.h>
#include <stdarg.h>

enum DEBUG_TYPE
{
	DEBUG_LEVEL_OFF = 0,
	DEBUG_LEVEL_FATAL = 1,
	DEBUG_LEVEL_ERROR = 2,
	DEBUG_LEVEL_WARN = 3,
	DEBUG_LEVEL_INFO = 4,
	DEBUG_LEVEL_DEBUG = 5,
	DEBUG_LEVEL_TRACE = 6
};

#ifndef      DEBUG_LEVEL
#define      DEBUG_LEVEL           DEBUG_LEVEL_TRACE
#endif

#define DEBUG(_DebugType, _Format, ...)																				\
do{																													\
	if (DEBUG_LEVEL >= _DebugType) {																				\
		printf(_Format, ##__VA_ARGS__);                																\
		printf("\n");                       																		\
		switch (_DebugType) {																						\
			case DEBUG_LEVEL_OFF:																					\
					break;                      																	\
			case DEBUG_LEVEL_FATAL:																					\
					printf("DEBUG : %s: %s: %u %s:%s\n", __FILE__, __func__, __LINE__, __DATE__, __TIME__); 		\
					break;                      																	\
			case DEBUG_LEVEL_ERROR:																					\
					printf("DEBUG : %s: %s: %u %s\n", __FILE__, __func__, __LINE__, __DATE__); 						\
					break;                      																	\
			case DEBUG_LEVEL_WARN:																					\
					printf("DEBUG : %s: %s\n", __FILE__, __func__); 												\
					break;                      																	\
			case DEBUG_LEVEL_INFO:																					\
					break;                      																	\
			case DEBUG_LEVEL_DEBUG:																					\
			case DEBUG_LEVEL_TRACE:																					\
					break;                      																	\
			default:																								\
					break;                     	 																	\
		}                                   																		\
	}																												\
}while (0)


#endif
