/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	public.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.24
描  述: 
备  注:	
修改记录:

  1.  日期: 2026.08.24
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#pragma once

#define PCI_TYPE0_ADDRESSES             6
#define PCI_TYPE1_ADDRESSES             2
#define PCI_TYPE2_ADDRESSES             5

// 只生成成员变量、get/set、Q_PROPERTY，不包含信号段
#define DECLARE_Q_PROPERTY(type, name, default_value) \
private: \
    type m_##name = default_value; \
public: \
    Q_INVOKABLE type get_##name() const { return m_##name; } \
    Q_INVOKABLE void set_##name(const type &val) { \
        if (m_##name != val) { \
            m_##name = val; \
            emit name##Changed(val); \
        } \
    } \
Q_PROPERTY(type name READ get_##name WRITE set_##name NOTIFY name##Changed)

// 放在类内部 signals: 下面，只声明信号，不要带Q_SIGNALS:
#define DECLARE_Q_PROPERTY_SIGNAL(type, name) \
    void name##Changed(const type &);
