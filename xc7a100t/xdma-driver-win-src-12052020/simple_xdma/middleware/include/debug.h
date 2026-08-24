/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	debug.h
作  者:	钱锐      版本: V1.1     新建日期: 2024.08.15
描  述: 调试日志文件
备  注:	支持linux和windows操作系统，支持纯C和C++，线程安全
修改记录:

  1.  日期: 2024.08.15
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

  2.  日期: 2026.08.16
      作者: 钱锐
      内容:
          1) 新增线程安全锁，保证多线程下日志不交错；
          2) 改为纯C兼容实现，移除对C++ std::mutex的依赖；
          3) Windows使用SRWLOCK，Linux使用pthread_mutex_t，均可静态初始化；
          4) 通过selectany(MSVC)/weak(GCC,Clang)实现跨翻译单元共享锁；
      版本:V1.1

*************************************************/

#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <stdio.h>
#include <stdarg.h>

/* ====== 跨翻译单元共享锁声明修饰符 ======
 * selectany(MSVC) / weak(GCC,Clang) 保证多个 .c/.cpp 文件共享同一个锁实例；
 * 其他编译器退化为 static（每翻译单元独立锁）。
 */
#if defined(_MSC_VER)
    #define DEBUG_LOCK_SHARED  __declspec(selectany)
#elif defined(__GNUC__) || defined(__clang__)
    #define DEBUG_LOCK_SHARED  __attribute__((weak))
#else
    #define DEBUG_LOCK_SHARED  static
#endif

/* ====== 平台相关的线程安全锁 ====== */
#ifdef _WIN32
    #include <windows.h>

    /* SRWLOCK 可静态初始化（SRWLOCK_INIT == {0}），无需显式调用初始化函数 */
    DEBUG_LOCK_SHARED SRWLOCK g_debug_lock = SRWLOCK_INIT;

    #define DEBUG_LOCK_ENTER() AcquireSRWLockExclusive(&g_debug_lock)
    #define DEBUG_LOCK_EXIT()  ReleaseSRWLockExclusive(&g_debug_lock)

#else
    #include <pthread.h>

    /* PTHREAD_MUTEX_INITIALIZER 支持静态初始化 */
    DEBUG_LOCK_SHARED pthread_mutex_t g_debug_lock = PTHREAD_MUTEX_INITIALIZER;

    #define DEBUG_LOCK_ENTER() pthread_mutex_lock(&g_debug_lock)
    #define DEBUG_LOCK_EXIT()  pthread_mutex_unlock(&g_debug_lock)
#endif

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
		DEBUG_LOCK_ENTER();																							\
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
		DEBUG_LOCK_EXIT();																							\
	}																												\
}while (0)


#endif
