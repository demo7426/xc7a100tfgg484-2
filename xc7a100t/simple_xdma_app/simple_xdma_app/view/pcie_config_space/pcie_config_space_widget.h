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
#include "base_view.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        class CPCIe_Config_Space_View_Model;
    }
}

class CPCIe_Config_Space_Widget : public QWidget, public hzcc::simple_xdma_app::CIBase_View
{
	Q_OBJECT

public:
    CPCIe_Config_Space_Widget(QWidget *parent = nullptr);
	~CPCIe_Config_Space_Widget();

    /// <summary>
    /// 初始化model
    /// </summary>
    /// <param name="model"></param>
    virtual void InitViewModel(std::shared_ptr<hzcc::simple_xdma_app::CIBase_View_Model> model) override;

private:
	Ui::CPCIe_Config_Space_WidgetClass ui;

    std::weak_ptr<hzcc::simple_xdma_app::CPCIe_Config_Space_View_Model> m_view_model;

private:
    /// <summary>
    /// 初始化ui界面
    /// </summary>
    void InitUi(void);

    /// <summary>
    /// 初始化信号槽
    /// </summary>
    void InitSignalSlots(void);

private:
};
