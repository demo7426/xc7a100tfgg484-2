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

#include <QMessageBox>

#define SOFT_VERSION ("V1.0")           //软件版本

simple_xdma_app::simple_xdma_app(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    this->InitData();
    this->InitUi();
    this->InitSignalSlots();
}

simple_xdma_app::~simple_xdma_app()
{}

void simple_xdma_app::InitData(void)
{
}

void simple_xdma_app::InitUi(void)
{
    QIcon winIcon(":/image/res/image/x.png");
    if (!winIcon.isNull())
    {
        setWindowIcon(winIcon);
    }

    QLabel* labelLeft = new QLabel("", this);

    auto widget = new CPCIe_Widget(this);
    setCentralWidget(widget);

    this->setWindowTitle(tr("Simple XDMA App"));

    this->resize(1280, 800);

    labelLeft->setText(tr("Card Num:") + QString::number(widget->GetCardNum()));

    ui.statusBar->addWidget(labelLeft);
}

void simple_xdma_app::InitSignalSlots(void)
{
    connect(ui.action_version, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, tr("Version information"), tr("Soft version: ") + SOFT_VERSION);
        });

    connect(ui.action_about_qt, &QAction::triggered, this, [this]() {
        QMessageBox::aboutQt(this);
        });
}
