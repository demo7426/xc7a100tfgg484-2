/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	pcie_config_space_view_model.cpp
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.25
描  述: pcie_config_space mdoel
备  注:
修改记录:

  1.  日期: 2026.08.25
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include "pcie_config_space_view_model.h"
#include "base_model.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        CPCIe_Config_Space_View_Model::CPCIe_Config_Space_View_Model(QObject* parent)
            : CIBase_View_Model(parent)
        {
        }

        CPCIe_Config_Space_View_Model::~CPCIe_Config_Space_View_Model()
        {
        }

        void CPCIe_Config_Space_View_Model::InitModel(std::shared_ptr<CIBase_Model> model)
        {
            if (auto old = m_pcieHeader.lock())
            {
                old->disconnect();
            }

            // 转型到 PCI_COMMON_HEADER（公共数据结构），不是具体 Model
            m_pcieHeader = std::dynamic_pointer_cast<PCI_COMMON_HEADER>(model);

            this->InitData();
            this->InitSignalSlots();
        }

        void CPCIe_Config_Space_View_Model::InitData(void)
        {
        }

        void CPCIe_Config_Space_View_Model::InitSignalSlots(void)
        {
            if (auto sp = m_pcieHeader.lock())
            {
                //链接信号槽
                connect(sp.get(), &PCI_COMMON_HEADER::VendorIDChanged, this, &CPCIe_Config_Space_View_Model::VendorIDChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::DeviceIDChanged, this, &CPCIe_Config_Space_View_Model::DeviceIDChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::CommandChanged, this, &CPCIe_Config_Space_View_Model::CommandChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::StatusChanged, this, &CPCIe_Config_Space_View_Model::StatusChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::RevisionIDChanged, this, &CPCIe_Config_Space_View_Model::RevisionIDChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::ProgIfChanged, this, &CPCIe_Config_Space_View_Model::ProgIfChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::SubClassChanged, this, &CPCIe_Config_Space_View_Model::SubClassChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::BaseClassChanged, this, &CPCIe_Config_Space_View_Model::BaseClassChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::CacheLineSizeChanged, this, &CPCIe_Config_Space_View_Model::CacheLineSizeChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::LatencyTimerChanged, this, &CPCIe_Config_Space_View_Model::LatencyTimerChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::HeaderTypeChanged, this, &CPCIe_Config_Space_View_Model::HeaderTypeChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::BISTChanged, this, &CPCIe_Config_Space_View_Model::BISTChanged);

                connect(sp.get(), &PCI_COMMON_HEADER::BarChanged, this, &CPCIe_Config_Space_View_Model::BarChanged);

                // ---- Type0 信号 ----
                connect(sp.get(), &PCI_COMMON_HEADER::CISChanged, this, &CPCIe_Config_Space_View_Model::CISChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::SubVendorIDChanged, this, &CPCIe_Config_Space_View_Model::SubVendorIDChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::SubSystemIDChanged, this, &CPCIe_Config_Space_View_Model::SubSystemIDChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::MinimumGrantChanged, this, &CPCIe_Config_Space_View_Model::MinimumGrantChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::MaximumLatencyChanged, this, &CPCIe_Config_Space_View_Model::MaximumLatencyChanged);

                // ---- Type1 信号 ----
                connect(sp.get(), &PCI_COMMON_HEADER::PrimaryBusChanged, this, &CPCIe_Config_Space_View_Model::PrimaryBusChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::SecondaryBusChanged, this, &CPCIe_Config_Space_View_Model::SecondaryBusChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::SubordinateBusChanged, this, &CPCIe_Config_Space_View_Model::SubordinateBusChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::SecondaryLatencyChanged, this, &CPCIe_Config_Space_View_Model::SecondaryLatencyChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::IOBaseChanged, this, &CPCIe_Config_Space_View_Model::IOBaseChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::IOLimitChanged, this, &CPCIe_Config_Space_View_Model::IOLimitChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::SecondaryStatusChanged, this, &CPCIe_Config_Space_View_Model::SecondaryStatusChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::MemoryBaseChanged, this, &CPCIe_Config_Space_View_Model::MemoryBaseChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::MemoryLimitChanged, this, &CPCIe_Config_Space_View_Model::MemoryLimitChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::PrefetchBaseChanged, this, &CPCIe_Config_Space_View_Model::PrefetchBaseChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::PrefetchLimitChanged, this, &CPCIe_Config_Space_View_Model::PrefetchLimitChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::PrefetchBaseUpper32Changed, this, &CPCIe_Config_Space_View_Model::PrefetchBaseUpper32Changed);
                connect(sp.get(), &PCI_COMMON_HEADER::PrefetchLimitUpper32Changed, this, &CPCIe_Config_Space_View_Model::PrefetchLimitUpper32Changed);
                connect(sp.get(), &PCI_COMMON_HEADER::IOBaseUpper16Changed, this, &CPCIe_Config_Space_View_Model::IOBaseUpper16Changed);
                connect(sp.get(), &PCI_COMMON_HEADER::IOLimitUpper16Changed, this, &CPCIe_Config_Space_View_Model::IOLimitUpper16Changed);
                connect(sp.get(), &PCI_COMMON_HEADER::BridgeControlChanged, this, &CPCIe_Config_Space_View_Model::BridgeControlChanged);

                // ---- 共用信号 ----
                connect(sp.get(), &PCI_COMMON_HEADER::ROMBaseAddressChanged, this, &CPCIe_Config_Space_View_Model::ROMBaseAddressChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::InterruptLineChanged, this, &CPCIe_Config_Space_View_Model::InterruptLineChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::InterruptPinChanged, this, &CPCIe_Config_Space_View_Model::InterruptPinChanged);
                connect(sp.get(), &PCI_COMMON_HEADER::CapabilitiesPtrChanged, this, &CPCIe_Config_Space_View_Model::CapabilitiesPtrChanged);

                connect(sp.get(), &PCI_COMMON_HEADER::allDataChanged, this, &CPCIe_Config_Space_View_Model::allDataChanged);
            }
        }

    }
}