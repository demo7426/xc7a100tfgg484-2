/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	base_info.h
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

#include <QObject>
#include <QMetaType>
#include <cstdint>

#include "types.h"
#include "public.h"

Q_DECLARE_METATYPE(int8_t)
Q_DECLARE_METATYPE(int16_t)
Q_DECLARE_METATYPE(int32_t)
Q_DECLARE_METATYPE(int64_t)

Q_DECLARE_METATYPE(uint8_t)
Q_DECLARE_METATYPE(uint16_t)
Q_DECLARE_METATYPE(uint32_t)
Q_DECLARE_METATYPE(uint64_t)

namespace hzcc
{
    namespace simple_xdma_app
    {
        enum class PCI_HEADER_TYPE : uint32_t
        {
            NONE = 0,
            DEVICE,         //EndPoint
            PCI_BRIDGE,     //PCO=to-PCI Bridge
            //CARDBUS,        //CardBus Bridge      //不支持，这种设备类型在2010年后，基本上被淘汰了
        };

        //PCIe 头部信息
        class PCI_COMMON_HEADER : public QObject 
        {
            Q_OBJECT

            DECLARE_Q_PROPERTY(uint16_t, VendorID, 0)
            DECLARE_Q_PROPERTY(uint16_t, DeviceID, 0)
            DECLARE_Q_PROPERTY(uint16_t, Command, 0)
            DECLARE_Q_PROPERTY(uint16_t, Status, 0)
            DECLARE_Q_PROPERTY(uint8_t, RevisionID, 0)
            DECLARE_Q_PROPERTY(uint8_t, ProgIf, 0)
            DECLARE_Q_PROPERTY(uint8_t, SubClass, 0)
            DECLARE_Q_PROPERTY(uint8_t, BaseClass, 0)
            DECLARE_Q_PROPERTY(uint8_t, CacheLineSize, 0)
            DECLARE_Q_PROPERTY(uint8_t, LatencyTimer, 0)
            DECLARE_Q_PROPERTY(uint8_t, HeaderType, 0)
            DECLARE_Q_PROPERTY(uint8_t, BIST, 0)

        public:
            PCI_COMMON_HEADER() = default;
            virtual ~PCI_COMMON_HEADER() {};

            //获取当前的 header 类型
            Q_INVOKABLE PCI_HEADER_TYPE GetActiveHeaderType() const
            {
                return static_cast<PCI_HEADER_TYPE>(m_HeaderType & 0x7F);
            }

            //获取bar
            Q_INVOKABLE QVariant Bar(int32_t index);

            //设置bar
            Q_INVOKABLE void SetBar(int32_t index, uint32_t value);

            Q_INVOKABLE uint32_t CIS() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::DEVICE) ? u.type0.CIS : 0;
            }

            Q_INVOKABLE uint16_t SubVendorID() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::DEVICE) ? u.type0.SubVendorID : 0;
            }

            Q_INVOKABLE uint16_t SubSystemID() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::DEVICE) ? u.type0.SubSystemID : 0;
            }

            Q_INVOKABLE uint8_t InterruptLine() const
            {
                switch (GetActiveHeaderType()) {
                case PCI_HEADER_TYPE::DEVICE:     return u.type0.InterruptLine;
                case PCI_HEADER_TYPE::PCI_BRIDGE: return u.type1.InterruptLine;
                default: return 0;
                }
            }

            Q_INVOKABLE uint8_t InterruptPin() const
            {
                switch (GetActiveHeaderType()) {
                case PCI_HEADER_TYPE::DEVICE:     return u.type0.InterruptPin;
                case PCI_HEADER_TYPE::PCI_BRIDGE: return u.type1.InterruptPin;
                default: return 0;
                }
            }

            Q_INVOKABLE uint32_t ROMBaseAddress() const
            {
                switch (GetActiveHeaderType()) {
                case PCI_HEADER_TYPE::DEVICE:     return u.type0.ROMBaseAddress;
                case PCI_HEADER_TYPE::PCI_BRIDGE: return u.type1.ROMBaseAddress;
                default: return 0;
                }
            }

            // ---- 共用（Type0 + Type1 都有）----
            Q_INVOKABLE uint8_t CapabilitiesPtr() const
            {
                switch (GetActiveHeaderType()) {
                case PCI_HEADER_TYPE::DEVICE:     return u.type0.CapabilitiesPtr;
                case PCI_HEADER_TYPE::PCI_BRIDGE: return u.type1.CapabilitiesPtr;
                default: return 0;
                }
            }

            // ---- Type0 独有 ----
            Q_INVOKABLE uint8_t MinimumGrant() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::DEVICE) ? u.type0.MinimumGrant : 0;
            }

            Q_INVOKABLE uint8_t MaximumLatency() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::DEVICE) ? u.type0.MaximumLatency : 0;
            }


            // ==================== Type1 访问器 ====================
            Q_INVOKABLE uint8_t PrimaryBus() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.PrimaryBus : 0;
            }

            Q_INVOKABLE uint8_t SecondaryBus() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.SecondaryBus : 0;
            }

            Q_INVOKABLE uint8_t SubordinateBus() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.SubordinateBus : 0;
            }

            Q_INVOKABLE uint8_t SecondaryLatency() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.SecondaryLatency : 0;
            }

            Q_INVOKABLE uint8_t IOBase() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.IOBase : 0;
            }

            Q_INVOKABLE uint8_t IOLimit() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.IOLimit : 0;
            }

            Q_INVOKABLE uint16_t SecondaryStatus() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.SecondaryStatus : 0;
            }

            Q_INVOKABLE uint16_t MemoryBase() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.MemoryBase : 0;
            }

            Q_INVOKABLE uint16_t MemoryLimit() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.MemoryLimit : 0;
            }

            Q_INVOKABLE uint16_t PrefetchBase() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.PrefetchBase : 0;
            }

            Q_INVOKABLE uint16_t PrefetchLimit() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.PrefetchLimit : 0;
            }

            Q_INVOKABLE uint32_t PrefetchBaseUpper32() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.PrefetchBaseUpper32 : 0;
            }

            Q_INVOKABLE uint32_t PrefetchLimitUpper32() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.PrefetchLimitUpper32 : 0;
            }

            Q_INVOKABLE uint16_t IOBaseUpper16() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.IOBaseUpper16 : 0;
            }

            Q_INVOKABLE uint16_t IOLimitUpper16() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.IOLimitUpper16 : 0;
            }

            Q_INVOKABLE uint16_t BridgeControl() const {
                return (GetActiveHeaderType() == PCI_HEADER_TYPE::PCI_BRIDGE) ? u.type1.BridgeControl : 0;
            }

            // ==================== Setter 声明 ====================

            // ---- Type0 setter ----
            Q_INVOKABLE void SetCIS(uint32_t value);
            Q_INVOKABLE void SetSubVendorID(uint16_t value);
            Q_INVOKABLE void SetSubSystemID(uint16_t value);
            Q_INVOKABLE void SetMinimumGrant(uint8_t value);
            Q_INVOKABLE void SetMaximumLatency(uint8_t value);

            // ---- Type1 setter ----
            Q_INVOKABLE void SetPrimaryBus(uint8_t value);
            Q_INVOKABLE void SetSecondaryBus(uint8_t value);
            Q_INVOKABLE void SetSubordinateBus(uint8_t value);
            Q_INVOKABLE void SetSecondaryLatency(uint8_t value);
            Q_INVOKABLE void SetIOBase(uint8_t value);
            Q_INVOKABLE void SetIOLimit(uint8_t value);
            Q_INVOKABLE void SetSecondaryStatus(uint16_t value);
            Q_INVOKABLE void SetMemoryBase(uint16_t value);
            Q_INVOKABLE void SetMemoryLimit(uint16_t value);
            Q_INVOKABLE void SetPrefetchBase(uint16_t value);
            Q_INVOKABLE void SetPrefetchLimit(uint16_t value);
            Q_INVOKABLE void SetPrefetchBaseUpper32(uint32_t value);
            Q_INVOKABLE void SetPrefetchLimitUpper32(uint32_t value);
            Q_INVOKABLE void SetIOBaseUpper16(uint16_t value);
            Q_INVOKABLE void SetIOLimitUpper16(uint16_t value);
            Q_INVOKABLE void SetBridgeControl(uint16_t value);

            // ---- 共用 setter ----
            Q_INVOKABLE void SetROMBaseAddress(uint32_t value);
            Q_INVOKABLE void SetInterruptLine(uint8_t value);
            Q_INVOKABLE void SetInterruptPin(uint8_t value);
            Q_INVOKABLE void SetCapabilitiesPtr(uint8_t value);

        Q_SIGNALS:
            DECLARE_Q_PROPERTY_SIGNAL(uint16_t, VendorID)
            DECLARE_Q_PROPERTY_SIGNAL(uint16_t, DeviceID)
            DECLARE_Q_PROPERTY_SIGNAL(uint16_t, Command)
            DECLARE_Q_PROPERTY_SIGNAL(uint16_t, Status)
            DECLARE_Q_PROPERTY_SIGNAL(uint8_t, RevisionID)
            DECLARE_Q_PROPERTY_SIGNAL(uint8_t, ProgIf)
            DECLARE_Q_PROPERTY_SIGNAL(uint8_t, SubClass)
            DECLARE_Q_PROPERTY_SIGNAL(uint8_t, BaseClass)
            DECLARE_Q_PROPERTY_SIGNAL(uint8_t, CacheLineSize)
            DECLARE_Q_PROPERTY_SIGNAL(uint8_t, LatencyTimer)
            DECLARE_Q_PROPERTY_SIGNAL(uint8_t, HeaderType)
            DECLARE_Q_PROPERTY_SIGNAL(uint8_t, BIST)

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

            void allDataChanged();   //整块数据变更通知
        private:

            union {
                struct _PCI_HEADER_TYPE_0 {
                    uint32_t   BaseAddresses[PCI_TYPE0_ADDRESSES];
                    uint32_t   CIS;
                    uint16_t  SubVendorID;
                    uint16_t  SubSystemID;
                    uint32_t   ROMBaseAddress;
                    uint8_t   CapabilitiesPtr;
                    uint8_t   Reserved1[3];
                    uint32_t   Reserved2;
                    uint8_t   InterruptLine;      //
                    uint8_t   InterruptPin;       // (ro)
                    uint8_t   MinimumGrant;       // (ro)
                    uint8_t   MaximumLatency;     // (ro)
                } type0;

                //
                // PCI to PCI Bridge
                //

                struct _PCI_HEADER_TYPE_1 {
                    uint32_t   BaseAddresses[PCI_TYPE1_ADDRESSES];
                    uint8_t   PrimaryBus;
                    uint8_t   SecondaryBus;
                    uint8_t   SubordinateBus;
                    uint8_t   SecondaryLatency;
                    uint8_t   IOBase;
                    uint8_t   IOLimit;
                    uint16_t  SecondaryStatus;
                    uint16_t  MemoryBase;
                    uint16_t  MemoryLimit;
                    uint16_t  PrefetchBase;
                    uint16_t  PrefetchLimit;
                    uint32_t   PrefetchBaseUpper32;
                    uint32_t   PrefetchLimitUpper32;
                    uint16_t  IOBaseUpper16;
                    uint16_t  IOLimitUpper16;
                    uint8_t   CapabilitiesPtr;
                    uint8_t   Reserved1[3];
                    uint32_t   ROMBaseAddress;
                    uint8_t   InterruptLine;
                    uint8_t   InterruptPin;
                    uint16_t  BridgeControl;
                } type1;

            } u;

        };
        
    }
}
