第1步: 骨架搭建
├── .vcxproj + .inf 项目配置
├── driver.c: DriverEntry + EvtDeviceAdd（空实现）
├── trace.h: WPP 跟踪宏
└── 确认驱动能加载/卸载

第2步: PCIe 基础
├── pcie.c/h: PciGetDeviceInfo, PciReadConfig
├── 在 EvtDevicePrepareHardware 中遍历 PCI 资源
└── 确认能读取 VID/DID

第3步: BAR 映射
├── bar.c/h: MapBARs, UnmapBARs, ReadReg32, WriteReg32
├── reg.h: 寄存器结构体定义
└── 确认能读写 config BAR 的 identifier 寄存器

第4步: 中断
├── interrupt.c/h: SetupInterrupts, ISR, DPC
├── 先只实现用户事件中断
└── 确认中断能触发

第5步: DMA 引擎
├── dma_engine.c/h: EngineCreate, EngineStart/Stop, ProgramDma
├── 先实现单通道 H2C
└── 确认数据能传输

第6步: 文件 IO
├── file_io.c/h: EvtDeviceFileCreate, EvtIoRead/Write
├── ioctl.c/h: IOCTL 处理
└── 确认用户态能读写

第7步: 多设备支持
├── 全局设备链表
├── 设备实例编号
└── 确认多卡能同时工作