/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	simple_xdma_app.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.21
描  述: 主窗口
备  注:	支持linux和windows操作系统
修改记录:

  1.  日期: 2026.08.21
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_simple_xdma_app.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        class CIProject_Model;
    }
}

class simple_xdma_app : public QMainWindow
{
    Q_OBJECT

public:
    simple_xdma_app(QWidget *parent = nullptr);
    ~simple_xdma_app();

private:
    Ui::simple_xdma_appClass ui;

    std::shared_ptr<hzcc::simple_xdma_app::CIProject_Model> m_cProject_Model;

private:
    /// <summary>
    /// 初始化数据
    /// </summary>
    /// <param name=""></param>
    void InitData(void);

    /// <summary>
    /// 初始化ui界面
    /// </summary>
    void InitUi(void);

    /// <summary>
    /// 初始化信号槽
    /// </summary>
    void InitSignalSlots(void);
};
