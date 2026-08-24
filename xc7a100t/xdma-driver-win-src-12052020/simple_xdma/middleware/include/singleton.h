/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	singleton.h
作  者:	钱锐      版本: V0.1.0     新建日期: 2025.05.06
描  述: 实现一个单例模板类
备  注:
修改记录:

  1.  日期: 2025.05.06
	  作者: 钱锐
	  内容:
		  1) 此为模板第一个版本；
	  版本:V0.1.0

*************************************************/

#pragma once

#include <iostream>
#include <memory>

template<typename T>
class CSingleton		//单例模板类
{
protected:
	CSingleton() = default;
	CSingleton(const CSingleton<T>&) = delete;
	CSingleton& operator = (const CSingleton<T>&) = delete;

public:
	~CSingleton()		//保证子类可以正常析构到父类
	{
		std::cout << __FUNCTION__ << std::endl;
	}

	/// <summary>
	/// 获取实例化对象
	/// </summary>
	/// <returns></returns>
	static std::shared_ptr<T> GetInstance()
	{
		static std::once_flag tOnceFlag;

		//std::call_once函数线程安全，其内部有加锁、解锁动作
		std::call_once(tOnceFlag, [&]() {
			m_pcInstance = std::shared_ptr<T>(new T);
			});

		return m_pcInstance;
	}

private:
	static std::shared_ptr<T> m_pcInstance;
};

template<typename T>
std::shared_ptr<T> CSingleton<T>::m_pcInstance = nullptr;


