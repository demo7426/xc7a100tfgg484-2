/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	pcie_config_space_widget.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.25
描  述: pcie配置空间
备  注:
修改记录:

  1.  日期: 2026.08.25
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#pragma once

#include <QWidget>

#include "ui_pcie_config_space_widget.h"

class CPCIe_Config_Space_Widget : public QWidget
{
	Q_OBJECT

public:
    CPCIe_Config_Space_Widget(QWidget *parent = nullptr);
	~CPCIe_Config_Space_Widget();

private:
	Ui::CPCIe_Config_Space_WidgetClass ui;

private:
    /// <summary>
    /// 初始化信号槽
    /// </summary>
    void InitSignalSlots(void);

private:
};
