/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	simple_xdma_app.cpp
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

#include "stdafx.h"
#include "simple_xdma_app.h"

#include "pcie_widget.h"

simple_xdma_app::simple_xdma_app(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    this->resize(1280, 800);

    setCentralWidget(new CPCIe_Widget(this));
}

simple_xdma_app::~simple_xdma_app()
{}
