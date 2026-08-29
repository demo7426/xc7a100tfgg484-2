/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	pcie_config_space_widget.cpp
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

#include "pcie_config_space_widget.h"
#include "pcie_config_space_view_model.h"

CPCIe_Config_Space_Widget::CPCIe_Config_Space_Widget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    this->InitUi();
}

CPCIe_Config_Space_Widget::~CPCIe_Config_Space_Widget()
{
}

void CPCIe_Config_Space_Widget::InitViewModel(std::shared_ptr<hzcc::simple_xdma_app::CIBase_View_Model> model)
{
	if (auto old = m_view_model.lock()) {
		old->disconnect(this);
	}

	m_view_model = std::dynamic_pointer_cast<hzcc::simple_xdma_app::CPCIe_Config_Space_View_Model>(model);

	if (m_view_model.lock()) {
		InitSignalSlots();    // ← 有 ViewModel 了才绑定
	}
}

void CPCIe_Config_Space_Widget::InitUi(void)
{
    ui.vendorIDSpinBox->setRange(0, UINT16_MAX);
    ui.deviceIDSpinBox->setRange(0, UINT16_MAX);
}

void CPCIe_Config_Space_Widget::InitSignalSlots(void)
{
    auto vm = m_view_model.lock();
    if (!vm) return;

    // 逐个字段绑定
    connect(vm.get(), &hzcc::simple_xdma_app::CPCIe_Config_Space_View_Model::VendorIDChanged,
        this, [this](uint16_t val) {
            ui.vendorIDSpinBox->setValue(val);
        });

    connect(vm.get(), &hzcc::simple_xdma_app::CPCIe_Config_Space_View_Model::DeviceIDChanged,
        this, [this](uint16_t val) {
            ui.deviceIDSpinBox->setValue(val);
        });

    // 批量刷新
    connect(vm.get(), &hzcc::simple_xdma_app::CPCIe_Config_Space_View_Model::allDataChanged,
        this, [this] {
            auto vm = m_view_model.lock();
            if (!vm) return;
            
            //ui.label_VendorID->setText(QString("0x%1").arg(vm->get_VendorID(), 4, 16, QChar('0')));
            // ... 全量刷新
        });
}
