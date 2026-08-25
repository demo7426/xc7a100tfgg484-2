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

#include "types.h"
#include "public.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        //PCIe 头部信息
        class PCI_COMMON_HEADER {
        public:
            PCI_COMMON_HEADER() = default;
            virtual ~PCI_COMMON_HEADER() {};

            USHORT  VendorID;                   // (ro)
            USHORT  DeviceID;                   // (ro)
            USHORT  Command;                    // Device control
            USHORT  Status;
            UCHAR   RevisionID;                 // (ro)
            UCHAR   ProgIf;                     // (ro)
            UCHAR   SubClass;                   // (ro)
            UCHAR   BaseClass;                  // (ro)
            UCHAR   CacheLineSize;              // (ro+)
            UCHAR   LatencyTimer;               // (ro+)
            UCHAR   HeaderType;                 // (ro)
            UCHAR   BIST;                       // Built in self test

            union {
                struct _PCI_HEADER_TYPE_0 {
                    ULONG   BaseAddresses[PCI_TYPE0_ADDRESSES];
                    ULONG   CIS;
                    USHORT  SubVendorID;
                    USHORT  SubSystemID;
                    ULONG   ROMBaseAddress;
                    UCHAR   CapabilitiesPtr;
                    UCHAR   Reserved1[3];
                    ULONG   Reserved2;
                    UCHAR   InterruptLine;      //
                    UCHAR   InterruptPin;       // (ro)
                    UCHAR   MinimumGrant;       // (ro)
                    UCHAR   MaximumLatency;     // (ro)
                } type0;



                //
                // PCI to PCI Bridge
                //

                struct _PCI_HEADER_TYPE_1 {
                    ULONG   BaseAddresses[PCI_TYPE1_ADDRESSES];
                    UCHAR   PrimaryBus;
                    UCHAR   SecondaryBus;
                    UCHAR   SubordinateBus;
                    UCHAR   SecondaryLatency;
                    UCHAR   IOBase;
                    UCHAR   IOLimit;
                    USHORT  SecondaryStatus;
                    USHORT  MemoryBase;
                    USHORT  MemoryLimit;
                    USHORT  PrefetchBase;
                    USHORT  PrefetchLimit;
                    ULONG   PrefetchBaseUpper32;
                    ULONG   PrefetchLimitUpper32;
                    USHORT  IOBaseUpper16;
                    USHORT  IOLimitUpper16;
                    UCHAR   CapabilitiesPtr;
                    UCHAR   Reserved1[3];
                    ULONG   ROMBaseAddress;
                    UCHAR   InterruptLine;
                    UCHAR   InterruptPin;
                    USHORT  BridgeControl;
                } type1;

                //
                // PCI to CARDBUS Bridge
                //

                struct _PCI_HEADER_TYPE_2 {
                    ULONG   SocketRegistersBaseAddress;
                    UCHAR   CapabilitiesPtr;
                    UCHAR   Reserved;
                    USHORT  SecondaryStatus;
                    UCHAR   PrimaryBus;
                    UCHAR   SecondaryBus;
                    UCHAR   SubordinateBus;
                    UCHAR   SecondaryLatency;
                    struct {
                        ULONG   Base;
                        ULONG   Limit;
                    }       Range[PCI_TYPE2_ADDRESSES - 1];
                    UCHAR   InterruptLine;
                    UCHAR   InterruptPin;
                    USHORT  BridgeControl;
                } type2;

            } u;

        };
        
    }
}
