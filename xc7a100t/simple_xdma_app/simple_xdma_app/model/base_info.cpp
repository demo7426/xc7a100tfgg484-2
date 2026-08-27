/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	base_info.cpp
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.27
描  述: 
备  注:	
修改记录:

  1.  日期: 2026.08.27
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include "base_info.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        Q_INVOKABLE QVariant PCI_COMMON_HEADER::Bar(int32_t index)
        {
            QVariant ret(QVariant::Invalid);
            auto type = GetActiveHeaderType();

            switch (type)
            {
            case hzcc::simple_xdma_app::PCI_HEADER_TYPE::NONE:
                break;
            case hzcc::simple_xdma_app::PCI_HEADER_TYPE::DEVICE:
            {
                if (index >= 0 && index < PCI_TYPE0_ADDRESSES)
                    ret.setValue(u.type0.BaseAddresses[index]);
            }
            break;
            case hzcc::simple_xdma_app::PCI_HEADER_TYPE::PCI_BRIDGE:
            {
                if (index >= 0 && index < PCI_TYPE1_ADDRESSES)
                    ret.setValue(u.type1.BaseAddresses[index]);
            }
            break;
            default:
                break;
            }

            return ret;
        }
        Q_INVOKABLE void PCI_COMMON_HEADER::SetBar(int32_t index, uint32_t value)
        {
            auto type = GetActiveHeaderType();

            switch (type)
            {
            case hzcc::simple_xdma_app::PCI_HEADER_TYPE::NONE:
                break;
            case hzcc::simple_xdma_app::PCI_HEADER_TYPE::DEVICE:
            {
                if (index >= 0 && index < PCI_TYPE0_ADDRESSES && u.type0.BaseAddresses[index] != value)
                {
                    u.type0.BaseAddresses[index] = value;
                    emit BarChanged(index, value);
                }
            }
            break;
            case hzcc::simple_xdma_app::PCI_HEADER_TYPE::PCI_BRIDGE:
            {
                if (index >= 0 && index < PCI_TYPE1_ADDRESSES && u.type1.BaseAddresses[index] != value)
                {
                    u.type1.BaseAddresses[index] = value;
                    emit BarChanged(index, value);
                }
            }
            break;
            default:
                break;
            }

            return;
        }

        // ==================== Type0 Setter ====================

        void PCI_COMMON_HEADER::SetCIS(uint32_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::DEVICE) return;
            if (u.type0.CIS != value) {
                u.type0.CIS = value;
                emit CISChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetSubVendorID(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::DEVICE) return;
            if (u.type0.SubVendorID != value) {
                u.type0.SubVendorID = value;
                emit SubVendorIDChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetSubSystemID(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::DEVICE) return;
            if (u.type0.SubSystemID != value) {
                u.type0.SubSystemID = value;
                emit SubSystemIDChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetMinimumGrant(uint8_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::DEVICE) return;
            if (u.type0.MinimumGrant != value) {
                u.type0.MinimumGrant = value;
                emit MinimumGrantChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetMaximumLatency(uint8_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::DEVICE) return;
            if (u.type0.MaximumLatency != value) {
                u.type0.MaximumLatency = value;
                emit MaximumLatencyChanged(value);
            }
        }

        // ==================== Type1 Setter ====================

        void PCI_COMMON_HEADER::SetPrimaryBus(uint8_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.PrimaryBus != value) {
                u.type1.PrimaryBus = value;
                emit PrimaryBusChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetSecondaryBus(uint8_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.SecondaryBus != value) {
                u.type1.SecondaryBus = value;
                emit SecondaryBusChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetSubordinateBus(uint8_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.SubordinateBus != value) {
                u.type1.SubordinateBus = value;
                emit SubordinateBusChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetSecondaryLatency(uint8_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.SecondaryLatency != value) {
                u.type1.SecondaryLatency = value;
                emit SecondaryLatencyChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetIOBase(uint8_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.IOBase != value) {
                u.type1.IOBase = value;
                emit IOBaseChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetIOLimit(uint8_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.IOLimit != value) {
                u.type1.IOLimit = value;
                emit IOLimitChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetSecondaryStatus(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.SecondaryStatus != value) {
                u.type1.SecondaryStatus = value;
                emit SecondaryStatusChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetMemoryBase(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.MemoryBase != value) {
                u.type1.MemoryBase = value;
                emit MemoryBaseChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetMemoryLimit(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.MemoryLimit != value) {
                u.type1.MemoryLimit = value;
                emit MemoryLimitChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetPrefetchBase(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.PrefetchBase != value) {
                u.type1.PrefetchBase = value;
                emit PrefetchBaseChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetPrefetchLimit(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.PrefetchLimit != value) {
                u.type1.PrefetchLimit = value;
                emit PrefetchLimitChanged(value);
            }
        }

        void PCI_COMMON_HEADER::SetPrefetchBaseUpper32(uint32_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.PrefetchBaseUpper32 != value) {
                u.type1.PrefetchBaseUpper32 = value;
                emit PrefetchBaseUpper32Changed(value);
            }
        }

        void PCI_COMMON_HEADER::SetPrefetchLimitUpper32(uint32_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.PrefetchLimitUpper32 != value) {
                u.type1.PrefetchLimitUpper32 = value;
                emit PrefetchLimitUpper32Changed(value);
            }
        }

        void PCI_COMMON_HEADER::SetIOBaseUpper16(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.IOBaseUpper16 != value) {
                u.type1.IOBaseUpper16 = value;
                emit IOBaseUpper16Changed(value);
            }
        }

        void PCI_COMMON_HEADER::SetIOLimitUpper16(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.IOLimitUpper16 != value) {
                u.type1.IOLimitUpper16 = value;
                emit IOLimitUpper16Changed(value);
            }
        }

        void PCI_COMMON_HEADER::SetBridgeControl(uint16_t value)
        {
            if (GetActiveHeaderType() != PCI_HEADER_TYPE::PCI_BRIDGE) return;
            if (u.type1.BridgeControl != value) {
                u.type1.BridgeControl = value;
                emit BridgeControlChanged(value);
            }
        }

        // ==================== 共用 Setter ====================

        void PCI_COMMON_HEADER::SetROMBaseAddress(uint32_t value)
        {
            switch (GetActiveHeaderType()) {
            case PCI_HEADER_TYPE::DEVICE:
                if (u.type0.ROMBaseAddress != value) {
                    u.type0.ROMBaseAddress = value;
                    emit ROMBaseAddressChanged(value);
                }
                break;
            case PCI_HEADER_TYPE::PCI_BRIDGE:
                if (u.type1.ROMBaseAddress != value) {
                    u.type1.ROMBaseAddress = value;
                    emit ROMBaseAddressChanged(value);
                }
                break;
            default: break;
            }
        }

        void PCI_COMMON_HEADER::SetInterruptLine(uint8_t value)
        {
            switch (GetActiveHeaderType()) {
            case PCI_HEADER_TYPE::DEVICE:
                if (u.type0.InterruptLine != value) {
                    u.type0.InterruptLine = value;
                    emit InterruptLineChanged(value);
                }
                break;
            case PCI_HEADER_TYPE::PCI_BRIDGE:
                if (u.type1.InterruptLine != value) {
                    u.type1.InterruptLine = value;
                    emit InterruptLineChanged(value);
                }
                break;
            default: break;
            }
        }

        void PCI_COMMON_HEADER::SetInterruptPin(uint8_t value)
        {
            switch (GetActiveHeaderType()) {
            case PCI_HEADER_TYPE::DEVICE:
                if (u.type0.InterruptPin != value) {
                    u.type0.InterruptPin = value;
                    emit InterruptPinChanged(value);
                }
                break;
            case PCI_HEADER_TYPE::PCI_BRIDGE:
                if (u.type1.InterruptPin != value) {
                    u.type1.InterruptPin = value;
                    emit InterruptPinChanged(value);
                }
                break;
            default: break;
            }
        }

        void PCI_COMMON_HEADER::SetCapabilitiesPtr(uint8_t value)
        {
            switch (GetActiveHeaderType()) {
            case PCI_HEADER_TYPE::DEVICE:
                if (u.type0.CapabilitiesPtr != value) {
                    u.type0.CapabilitiesPtr = value;
                    emit CapabilitiesPtrChanged(value);
                }
                break;
            case PCI_HEADER_TYPE::PCI_BRIDGE:
                if (u.type1.CapabilitiesPtr != value) {
                    u.type1.CapabilitiesPtr = value;
                    emit CapabilitiesPtrChanged(value);
                }
                break;
            default: break;
            }
        }
    }
}
