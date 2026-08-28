/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	pcie_config_space_view_model.h
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

#pragma once

#include <QObject>
#include <memory>

#include "base_info.h"      // PCI_COMMON_HEADER —— 协议层定义，公共依赖
#include "base_view_model.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        class CIBase_Model;

        //TODO:CPCIe_Config_Space_View_Model 暂时只读 Model 数据，因为底层驱动还暂未支持设置接口
        class CPCIe_Config_Space_View_Model : public CIBase_View_Model
        {
            Q_OBJECT

        public:
            CPCIe_Config_Space_View_Model(QObject* parent = nullptr);
            ~CPCIe_Config_Space_View_Model();

            /// <summary>
            /// 初始化model
            /// </summary>
            /// <param name="model"></param>
            virtual void InitModel(std::shared_ptr<CIBase_Model> model) override;

        private:
            /// <summary>
            /// 初始化数据
            /// </summary>
            /// <param name=""></param>
            void InitData(void);

            /// <summary>
            /// 初始化信号槽
            /// </summary>
            void InitSignalSlots(void);

        Q_SIGNALS:
            void VendorIDChanged(uint16_t);    
            void DeviceIDChanged(uint16_t);
            void CommandChanged(uint16_t);
            void StatusChanged(uint16_t);
            void RevisionIDChanged(uint8_t);
            void ProgIfChanged(uint8_t);
            void SubClassChanged(uint8_t);
            void BaseClassChanged(uint8_t);
            void CacheLineSizeChanged(uint8_t);
            void LatencyTimerChanged(uint8_t);
            void HeaderTypeChanged(uint8_t);
            void BISTChanged(uint8_t);

            void BarChanged(int32_t index, uint32_t value);

            // ---- Type0 信号 ----
            void CISChanged(uint32_t value);
            void SubVendorIDChanged(uint16_t value);
            void SubSystemIDChanged(uint16_t value);
            void MinimumGrantChanged(uint8_t value);
            void MaximumLatencyChanged(uint8_t value);

            // ---- Type1 信号 ----
            void PrimaryBusChanged(uint8_t value);
            void SecondaryBusChanged(uint8_t value);
            void SubordinateBusChanged(uint8_t value);
            void SecondaryLatencyChanged(uint8_t value);
            void IOBaseChanged(uint8_t value);
            void IOLimitChanged(uint8_t value);
            void SecondaryStatusChanged(uint16_t value);
            void MemoryBaseChanged(uint16_t value);
            void MemoryLimitChanged(uint16_t value);
            void PrefetchBaseChanged(uint16_t value);
            void PrefetchLimitChanged(uint16_t value);
            void PrefetchBaseUpper32Changed(uint32_t value);
            void PrefetchLimitUpper32Changed(uint32_t value);
            void IOBaseUpper16Changed(uint16_t value);
            void IOLimitUpper16Changed(uint16_t value);
            void BridgeControlChanged(uint16_t value);

            // ---- 共用信号 ----
            void ROMBaseAddressChanged(uint32_t value);
            void InterruptLineChanged(uint8_t value);
            void InterruptPinChanged(uint8_t value);
            void CapabilitiesPtrChanged(uint8_t value);


            void allDataChanged(void);     //所有数据发生了改变

        private:
            std::weak_ptr<PCI_COMMON_HEADER> m_pcieHeader;
        };

    }
}
