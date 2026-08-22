# XDMA Windows 驱动架构文档

> Xilinx DMA/Bridge Subsystem for PCIe (XDMA) — Windows WDF 内核驱动
> 项目根目录：`simple_xdma\driver\simple_xdma\`
> 适用 IP 版本：2017.1（兼容到 2017.3）

---

## 1. 项目概览

本驱动基于 Windows Driver Framework (KMDF) 实现，用于驱动 Xilinx XDMA PCIe IP 核，提供：

- **DMA 传输**：H2C（Host→Card）/ C2H（Card→Host）的 Scatter-Gather DMA
- **寄存器访问**：通过用户态文件句柄直接读写 PCIe BAR 映射内存
- **用户事件**：16 路用户中断事件，支持阻塞等待
- **Streaming DMA**：基于 Ring Buffer 的 AXI-ST 流式数据传输
- **Poll 模式**：可选的轮询式完成检测（绕过中断）
- **性能计数**：引擎性能计数器读写
- **BAR 映射**：把 BAR 内核地址映射到用户态（MmMapLockedPagesSpecifyCache）

---

## 2. 文件清单

```
simple_xdma\
├── driver.c             # WDF 驱动入口、设备/队列初始化
├── driver.h             # DeviceContext 设备上下文
├── device.c             # XDMA 设备核心：BAR 映射、寄存器解析、引擎探测入口
├── device.h             # XDMA_DEVICE 结构、XDMA_EVENT 结构
├── dma_engine.c         # DMA 引擎实现：描述符、传输、Ring、Poll、性能计数
├── dma_engine.h         # XDMA_ENGINE 结构、Ring/Descriptor/描述符结构
├── interrupt.c          # 中断建立与 ISR/DPC 处理（Line/MSI/MSI-X）
├── interrupt.h          # IRQ_CONTEXT 结构、SetupInterrupts 声明
├── pcie_common.h        # PCI 配置空间读取工具（MSI/MSI-X/Line 探测）
├── reg.h                # 所有寄存器布局定义、宏、block offset
├── xdma.h               # 公共 API 声明（XDMA_DeviceOpen/Close 等）
├── xdma_public.h        # 设备接口 GUID、文件名宏、IOCTL 码、用户结构体
├── file_io.c            # 文件对象/IO 队列回调，用户请求分发
├── file_io.h            # FILE_CONTEXT、QUEUE_CONTEXT、DevNode 类型
├── trace.h              # WPP 跟踪宏定义、TraceVerbose/Info/Warning/Error
├── simple_xdma.inf      # Windows 安装信息文件
├── simple_xdma.inx      # 通用 INF 模板（构建期生成 .inf）
├── simple_xdma.vcxproj  # Visual Studio 项目
└── *.vcxproj.filters/user # VS 项目辅助文件
```

---

## 3. 模块划分

| 模块 | 文件 | 职责 |
|---|---|---|
| **WDF 框架入口** | `driver.c` `driver.h` | DriverEntry、EvtDeviceAdd、EvtDevicePrepareHardware/ReleaseHardware、EngineCreateQueue |
| **设备核心** | `device.c` `device.h` | XDMA_DeviceOpen/Close、MapBARs、IdentifyBars、GetRegisterModules、GetVersion |
| **DMA 引擎** | `dma_engine.c` `dma_engine.h` | ProbeEngines、EngineCreate、XDMA_EngineProgramDma、EngineStart/Stop、EngineProcessTransfer/Ring、Poll、Ring buffer、性能计数 |
| **中断管理** | `interrupt.c` `interrupt.h` | SetupInterrupts（Line/MSI/MSI-X）、EvtInterruptIsr/Dpc、EvtChannelInterruptIsr/Dpc、EvtUserInterruptIsr/Dpc |
| **PCI 配置空间** | `pcie_common.h` | GetNumMsiVectors、GetNumMsixVectors、GetLineInterruptPin、FindCapability |
| **寄存器定义** | `reg.h` | 所有寄存器结构体（XDMA_CONFIG_REGS/IRQ_REGS/ENGINE_REGS/SGDMA_REGS）与 block offset 常量 |
| **公共接口** | `xdma.h` `xdma_public.h` | XDMA_DeviceOpen/Close/UserIsrRegister 公共 API、设备接口 GUID、文件名宏、IOCTL 码、XDMA_PERF_DATA 等 |
| **文件 IO** | `file_io.c` `file_io.h` | EvtDeviceFileCreate/Close/Cleanup、EvtIoRead/Write/DeviceControl、EvtIoReadDma/WriteDma/ReadEngineRing、EvtReadUserEvent、HandleUserEvent、IoctlBar/IoctlChannel/IoctlMapBar/IoctlKeyholeWriteRegister |
| **跟踪** | `trace.h` | WPP_CONTROL_GUIDS、TraceVerbose/Info/Warning/Error 宏、Release 模式 stub |
| **构建/安装** | `simple_xdma.inf/.inx/.vcxproj` | 安装信息、KMDF 服务注册、Visual Studio 构建配置 |

---

## 4. 顶层架构图

```
+-------------------------------------------------------------+
|                     User-Space Application                  |
|   CreateFile("\\h2c_0"), ReadFile, WriteFile, DeviceIoControl|
+-----------------------------+-------------------------------+
                              | Win32 API
                              v
+-------------------------------------------------------------+
|                       Windows I/O Manager                   |
|         (IRP / WDFREQUEST, PnP 资源 ResourcesRaw/Translated) |
+-----------------------------+-------------------------------+
                              | WDF
                              v
+-------------------------------------------------------------+
|                  simple_xdma.sys (KMDF Driver)              |
|                                                             |
|  +-------------------+        +---------------------------+  |
|  | WDF 框架入口       |        | 文件 IO 模块 (file_io.c)   |  |
|  | driver.c          |------->|  EvtIoRead/Write/DeviceCtl |  |
|  | DriverEntry       |        |  EvtDeviceFileCreate       |  |
|  | EvtDeviceAdd      |        +------------+--------------+  |
|  | EvtDevicePrepareHw|                    |                 |
|  +--------+----------+                    v                 |
|           |            +---------------------------+        |
|           |            | DMA 引擎模块 (dma_engine.c)|        |
|           v            |  EngineProgramDma         |        |
|  +----------------+   |  EngineProcessTransfer/Ring|        |
|  | 设备核心模块     |   |  Poll/Perf                |        |
|  | device.c       |   +------------+--------------+        |
|  | XDMA_DeviceOpen|                |                      |
|  | MapBARs        |                v                      |
|  | IdentifyBars   |   +---------------------------+      |
|  | GetRegisterMods |   | 中断模块 (interrupt.c)     |      |
|  +-------+--------+   |  EvtInterruptIsr/Dpc       |      |
|          |            |  EvtChannel/User Interrupt |      |
|          |            +------------+----------------+      |
|          v                         |                      |
|  +----------------+                |                      |
|  | 寄存器/PCIe      |<---------------+                      |
|  | pcie_common.h   |                                       |
|  | reg.h           |                                       |
|  +----------------+                                       |
+-------------------------------------------------------------+
                              | PCIe config + BAR MMIO
                              v
+-------------------------------------------------------------+
|              Xilinx XDMA PCIe IP Core (FPGA)                |
|   BAR0 (user)  BAR1 (config)  BAR2 (bypass)                |
|   H2C/C2H Engines  IRQ Regs  Config Regs  SGDMA Regs       |
+-------------------------------------------------------------+
```

---

## 5. 启动与初始化调用链

### 5.1 驱动加载链

```
DriverEntry (driver.c:130)
├── WPP_INIT_TRACING                           # 启用 WPP 跟踪
├── WDF_DRIVER_CONFIG_INIT(&DriverConfig, EvtDeviceAdd)
└── WdfDriverCreate                            # 创建 WDFDRIVER 对象
    └── (PnP 发现设备时回调) EvtDeviceAdd (driver.c:178)
        ├── WdfDeviceInitSetIoType(Direct)       # 选用 Direct I/O
        ├── WDF_PNPPOWER_EVENT_CALLBACKS_INIT    # 注册 PnP/Power 回调
        │   ├── .EvtDevicePrepareHardware = EvtDevicePrepareHardware
        │   └── .EvtDeviceReleaseHardware = EvtDeviceReleaseHardware
        ├── WDF_FILEOBJECT_CONFIG_INIT          # 文件对象回调
        │   ├── .EvtDeviceFileCreate = EvtDeviceFileCreate
        │   ├── .EvtFileClose = EvtFileClose
        │   └── .EvtFileCleanup = EvtFileCleanup
        ├── WdfDeviceInitSetIoInCallerContextCallback(EvtDeviceIoInCallerContext)
        ├── WdfDeviceCreate                     # 创建 WDFDEVICE
        ├── WdfDeviceCreateDeviceInterface(GUID_DEVINTERFACE_XDMA)
        └── WdfIoQueueCreate (Default Queue, Parallel)
            ├── .EvtIoDeviceControl = EvtIoDeviceControl
            ├── .EvtIoRead = EvtIoRead
            └── .EvtIoWrite = EvtIoWrite
```

### 5.2 设备硬件准备链

```
EvtDevicePrepareHardware (driver.c:266)
├── XDMA_DeviceOpen(device, xdma, &userMax, &h2cChMax, &c2hChMax,
│                   ResourcesRaw, ResourcesTranslated)   # device.c:316
│   ├── DeviceDefaultInitialize(xdma)                     # device.c:116 清零所有字段
│   ├── xdma->wdfDevice = wdfDevice
│   ├── MapBARs(xdma, ResourcesTranslated)                # device.c:152
│   │   ├── GetRealBarIndices(...)                         # device.c:153 读 PCI 配置空间恢复真实 BAR 编号
│   │   │   ├── WdfFdoQueryForInterface(GUID_BUS_INTERFACE_STANDARD)
│   │   │   └── pciBus.GetBusData(PCI_WHICHSPACE_CONFIG, &pciHeader, 0, 64)
│   │   ├── for 每个 CmResourceTypeMemory 资源:
│   │   │   ├── xdma->barLength[realIdx] = resource->u.Memory.Length
│   │   │   └── xdma->bar[realIdx] = MmMapIoSpace(Start, Length, MmNonCached)
│   │   └── xdma->numBars++
│   ├── IdentifyBars(xdma)                                # device.c:278
│   │   ├── _QIANRUI 分支：硬编码 configBarIdx/userBarIdx/bypassBarIdx
│   │   └── 默认分支：
│   │       ├── FindConfigBAR → IsConfigBAR (读 IRQ/CONFIG identifier 比对 XDMA_ID)
│   │       ├── userBarIdx  = configBarIdx==1 ? 0 : -1
│   │       └── bypassBarIdx = (numBars - configBarIdx == 2) ? numBars-1 : -1
│   ├── GetRegisterModules(xdma)                          # device.c:307
│   │   ├── configRegs   = bar[configBarIdx] + CONFIG_BLOCK_OFFSET
│   │   ├── interruptRegs = bar[configBarIdx] + IRQ_BLOCK_OFFSET
│   │   └── sgdmaRegs    = bar[configBarIdx] + SGDMA_COMMON_BLOCK_OFFSET
│   ├── GetVersion(xdma)                                  # 检查 IP 版本是否匹配 2017.1
│   ├── SetupInterrupts(xdma, &userMax, &h2cChMax, &c2hChMax,
│   │                     ResourcesRaw, ResourcesTranslated)  # interrupt.c:590
│   │   ├── CountChannels(xdma, ...)                     # 通过读 engineRegs.identifier 探测 H2C/C2H 通道数
│   │   ├── GetNumMsixVectors(device, ...)                # pcie_common.h:146 读 PCI 配置空间 MSI-X cap
│   │   ├── GetNumMsiVectors(device, ...)                # pcie_common.h:109 读 PCI 配置空间 MSI cap
│   │   ├── CountInterruptResources(ResourcesTranslated, ...)
│   │   └── 按 MSIX / Multi-MSI / Line 三种模式分别调用：
│   │       ├── SetupMsixInterrupts     → SetupUserInterrupt / SetupChannelInterrupt
│   │       ├── SetupMultiMsiInterrupts → SetupUserInterrupt / SetupChannelInterrupt
│   │       └── SetupSingleInterrupt    → SetupDeviceInterrupt
│   │   └── programInterrupts(xdma, ...)                  # 写 interruptRegs->userVector/channelVector
│   ├── WdfDeviceSetAlignmentRequirement(device, 8-1)
│   ├── WDF_DMA_ENABLER_CONFIG_INIT(WdfDmaProfileScatterGather64Duplex, XDMA_MAX_TRANSFER_SIZE)
│   ├── WdfDmaEnablerCreate(device, &dmaConfig, ..., &xdma->dmaEnabler)
│   └── ProbeEngines(xdma)                                # dma_engine.c:778
│       ├── CountChannels(xdma, &h2cChMax, &c2hChMax)
│       └── for H2C 然后 C2H，每个通道：
│           if EngineExists(xdma, dir, ch):               # 读 engineRegs.identifier
│               EngineCreate(xdma, engine, dir, ch, engineIndex, engineId)  # dma_engine.c:487
│               ├── engine->regs   = configBarAddr + dir*BLOCK_OFFSET + ch*ENGINE_OFFSET
│               ├── engine->sgdma  = configBarAddr + dir*BLOCK_OFFSET + ch*ENGINE_OFFSET + SGDMA_BLOCK_OFFSET
│               ├── engine->type   = (identifier & XDMA_ID_ST_BIT) ? ST : MM
│               ├── engine->addressMode = (control & XDMA_CTRL_NON_INCR_ADDR) ? Fixed : Contiguous
│               ├── EngineConfigureInterrupt(engine, engineIndex, engineId)
│               │   ├── engine->irqBitMask = (1<<XDMA_ENG_IRQ_NUM)-1 << (engineId*XDMA_ENG_IRQ_NUM)
│               │   ├── irqContext->engine = engine  # 把 IRQ context 与 engine 绑定
│               │   └── engine->regs->intEnableMaskW1S = XDMA_CTRL_IE_ALL
│               ├── EngineCreatePollWriteBackBuffer(engine)  # 分配 poll 完成 buffer
│               ├── EngineGetAlignments(engine)              # 读 alignments 寄存器
│               ├── EngineCreateDescriptorBuffer(engine)    # 分配 WDFCOMMONBUFFER 给描述符
│               │   ├── WdfCommonBufferCreate(dmaEnabler, bufferSize, ...)
│               │   ├── engine->capacity = bufferSize / sizeof(DMA_DESCRIPTOR)
│               │   └── engine->sgdma->firstDescLo/Hi = descBuffer 的物理地址
│               ├── WdfDmaTransactionCreate(dmaEnabler, ..., &engine->dmaTransaction)
│               ├── if (type==ST && dir==C2H):
│               │   ├── engine->work = EngineProcessRing
│               │   ├── EngineCreateRingBuffer(engine)      # dma_engine.c:881
│               │   │   ├── WdfCommonBufferCreate for results
│               │   │   ├── WdfCommonBufferCreateWithConfig for receiveBuffer (258 块 × 4KB)
│               │   │   ├── for i in 0..258: 填充 xmdl[i].virtAddr/dmaAdrr/len
│               │   │   ├── WdfSpinLockCreate(&engine->ring.lock)
│               │   │   └── KeInitializeEvent(&engine->ring.completionSignal)
│               │   └── sgdmaRegs->creditModeEnableW1S = BIT_N(ch)<<16
│               ├── else: engine->work = EngineProcessTransfer
│               ├── engine->isReqPending = FALSE
│               ├── WdfSpinLockCreate(&engine->engineLock)
│               ├── KeInitializeEvent(&engine->completionWaitSignal)
│               ├── initThread(engine)                     # dma_engine.c:434 创建系统线程
│               │   ├── KeInitializeSemaphore(&engine->semaphore, 0, MAXLONG)
│               │   ├── PsCreateSystemThread(..., xdmaCompletionThread, engine)
│               │   ├── ObReferenceObjectByHandle(... &engine->thObject)
│               │   └── engine->thInitialized = TRUE
│               └── engine->enabled = TRUE
│
├── GetPollModeParameter(&pollMode)                       # driver.c:100 读注册表 POLL_MODE
├── for 每个引擎: XDMA_EngineSetPollMode(engine, pollMode)  # dma_engine.c:1356
├── for 每个启用引擎: EngineCreateQueue(device, engine, &ctx->engineQueue[dir][ch])  # driver.c:339
│   ├── WDF_IO_QUEUE_CONFIG_INIT(Sequential)
│   ├── if H2C: config.EvtIoWrite = EvtIoWriteDma
│   ├── if C2H (ST): config.EvtIoRead = EvtIoReadEngineRing
│   ├── if C2H (MM): config.EvtIoRead = EvtIoReadDma
│   ├── WdfIoQueueCreate(...)
│   └── queueContext->engine = engine
└── for i in 0..XDMA_MAX_USER_IRQ:
    ├── KeInitializeEvent(&ctx->eventSignals[i], NotificationEvent, FALSE)
    └── XDMA_UserIsrRegister(xdma, i, HandleUserEvent, &ctx->eventSignals[i])  # interrupt.c:668
```

### 5.3 设备释放链

```
EvtDeviceReleaseHardware (driver.c:324)
└── XDMA_DeviceClose(&ctx->xdma)                          # device.c:329
    ├── if (interruptRegs):                                # 清零 IRQ 向量寄存器
    │   ├── userVector[0..3] = 0
    │   └── channelVector[0..1] = 0
    ├── closeEngines(xdma)                                 # dma_engine.c:829
    │   └── for H2C/C2H 所有引擎: terminateThread(engine)  # dma_engine.c:466
    │       ├── engine->terminate = TRUE
    │       ├── KeReleaseSemaphore(&engine->semaphore, ...)
    │       ├── KeWaitForSingleObject(engine->thObject, ...)  # 等线程退出
    │       └── ObDereferenceObject(engine->thObject)
    └── for i in 0..numBars: MmUnmapIoSpace(bar[i], barLength[i])
```

---

## 6. 数据结构层级

```
WDFDRIVER (driver 全局)
└── WDFDEVICE (一个物理设备)
    └── DeviceContext (driver.h:65)  [WdfDevice context]
        ├── XDMA_DEVICE xdma (device.h:88)
        │   ├── WDFDEVICE wdfDevice
        │   ├── UINT numBars
        │   ├── PVOID bar[3]                         ← BAR0/1/2 内核虚拟地址
        │   ├── ULONG barLength[3]
        │   ├── ULONG configBarIdx
        │   ├── LONG  userBarIdx / bypassBarIdx
        │   ├── volatile XDMA_CONFIG_REGS*       configRegs
        │   ├── volatile XDMA_IRQ_REGS*          interruptRegs
        │   ├── volatile XDMA_SGDMA_COMMON_REGS* sgdmaRegs
        │   ├── XDMA_ENGINE engines[4][2]            ← [channel][dir]
        │   │   ├── volatile XDMA_ENGINE_REGS*  regs
        │   │   ├── volatile XDMA_SGDMA_REGS*   sgdma
        │   │   ├── UINT32 irqBitMask
        │   │   ├── UINT32 alignAddr/alignLength/alignAddrBits
        │   │   ├── DWORD channel, DirToDev dir
        │   │   ├── BOOLEAN enabled
        │   │   ├── EngineType type (MM/ST)
        │   │   ├── AddressMode addressMode (Contiguous/Fixed)
        │   │   ├── UINT32 capacity
        │   │   ├── volatile BOOLEAN isReqPending
        │   │   ├── WDFSPINLOCK engineLock
        │   │   ├── WDFCOMMONBUFFER descBuffer     ← DMA 描述符缓冲
        │   │   ├── WDFDMATRANSACTION dmaTransaction
        │   │   ├── PFN_XDMA_ENGINE_WORK work      ← 完成回调 (EngineProcessTransfer / Ring)
        │   │   ├── XDMA_RING ring                  ← 仅 ST 引擎
        │   │   │   ├── WDFCOMMONBUFFER results
        │   │   │   ├── WDFCOMMONBUFFER receiveBuffer
        │   │   │   ├── XMDL xmdl[258]
        │   │   │   │   ├── WDFCOMMONBUFFER rcvBuffer
        │   │   │   │   ├── PVOID virtAddr
        │   │   │   │   ├── PHYSICAL_ADDRESS dmaAdrr
        │   │   │   │   └── size_t len
        │   │   │   ├── CHAR dmaTransferContext[DMA_TRANSFER_CONTEXT_SIZE_V1]
        │   │   │   ├── UINT head/tail
        │   │   │   ├── WDFSPINLOCK lock
        │   │   │   └── KEVENT completionSignal
        │   │   ├── ULONG poll
        │   │   ├── WDFCOMMONBUFFER pollWbBuffer
        │   │   ├── ULONG numDescriptors
        │   │   ├── KEVENT completionWaitSignal
        │   │   ├── BOOLEAN thInitialized
        │   │   ├── void* thObject
        │   │   ├── KSEMAPHORE semaphore
        │   │   ├── HANDLE thHandle
        │   │   └── BOOLEAN terminate
        │   ├── WDFDMAENABLER dmaEnabler
        │   ├── WDFINTERRUPT lineInterrupt              ← legacy INTx
        │   ├── WDFINTERRUPT channelInterrupts[8]       ← MSI/MSI-X 通道 IRQ
        │   ├── XDMA_EVENT userEvents[16]               ← 16 路用户事件
        │   │   ├── PFN_XDMA_USER_WORK work
        │   │   ├── void* userData
        │   │   └── WDFINTERRUPT irq
        │   └── ULONG userMax/h2cChannelMax/c2hChannelMax
        ├── WDFQUEUE engineQueue[2][4]                  ← 每方向每通道一个引擎队列
        └── KEVENT eventSignals[16]                     ← 用户事件等待信号

WDFINTERRUPT
└── IRQ_CONTEXT (interrupt.h:72) [WdfInterrupt context]
    ├── ULONG eventId
    ├── UINT32 channelIrqPending                       ← ISR 记录已触发通道位
    ├── UINT32 userIrqPending                          ← ISR 记录已触发用户位
    ├── XDMA_ENGINE* engine                            ← 仅通道 IRQ 绑定
    ├── volatile XDMA_IRQ_REGS* regs
    └── PXDMA_DEVICE xdma

WDFFILEOBJECT
└── FILE_CONTEXT (file_io.h:81) [WdfFileObject context]
    ├── DEVNODE_TYPE devType                           ← USER/CONTROL/BYPASS/H2C/C2H/EVENTS
    ├── union { void* bar | XDMA_EVENT* event | XDMA_ENGINE* engine } u
    ├── WDFQUEUE queue                                  ← 引擎队列
    ├── PMDL mdl                                       ← 用于 IOCTL_MAP_BAR 用户态映射
    └── PVOID virtAddress                              ← 用户态映射地址

WDFQUEUE
└── QUEUE_CONTEXT (file_io.h:95) [WdfQueue context]
    └── XDMA_ENGINE* engine                            ← 引擎队列绑定的引擎
```

---

## 7. BAR 与寄存器布局

### 7.1 BAR 角色与识别

```
+-----------------+----------------+----------------+
| BAR Index       | 角色           | 识别方式       |
+-----------------+----------------+----------------+
| configBarIdx    | 配置 BAR       | IsConfigBAR:   |
|                 | 含 IRQ/Config/ | 读 IRQ_BLOCK   |
|                 | SGDMA/Engine   | + CONFIG_BLOCK |
|                 | 寄存器         | identifier ==  |
|                 |                | XDMA_ID        |
+-----------------+----------------+----------------+
| userBarIdx      | 用户 BAR       | 通常 = BAR0    |
| (可不存在)       | 用户 AXI-Lite  | (当 config=1) |
+-----------------+----------------+----------------+
| bypassBarIdx    | 旁路 BAR       | 当 numBars -  |
| (可不存在)       | 用户直接写描述符| configBarIdx  |
|                 |                | == 2 时存在   |
+-----------------+----------------+----------------+
```

`device.c:152` 中 `GetRealBarIndices` 通过读 PCI 配置空间 `u.type0.BaseAddresses[0..5]` 恢复真实物理 BAR 编号（区分 32-bit / 64-bit BAR），让 `bar[]` 数组下标 = 物理 BAR 号。

### 7.2 配置 BAR 内的寄存器 block 布局

```
configBarAddr + 0x0000   engineRegs[0][H2C]      (H2C channel 0)
            + 0x0100   engineRegs[1][H2C]      (H2C channel 1)  ← ENGINE_OFFSET = 0x100
            + ...
            + 0x1000   engineRegs[0][C2H]      (C2H channel 0)  ← BLOCK_OFFSET = 0x1000
            + 0x1100   engineRegs[1][C2H]      (C2H channel 1)
            + ...
            + 0x2000   IRQ_BLOCK_OFFSET         ← XDMA_IRQ_REGS
            + 0x3000   CONFIG_BLOCK_OFFSET      ← XDMA_CONFIG_REGS
            + 0x4000   SGDMA_BLOCK_OFFSET       ← H2C XDMA_SGDMA_REGS (per channel)
            + 0x5000   SGDMA_BLOCK_OFFSET       ← C2H XDMA_SGDMA_REGS (per channel)
            + 0x6000   SGDMA_COMMON_BLOCK_OFFSET ← XDMA_SGDMA_COMMON_REGS
```

每个 `XDMA_ENGINE` 同时持有：
- `regs`  = configBarAddr + dir*BLOCK_OFFSET + ch*ENGINE_OFFSET               → `XDMA_ENGINE_REGS`
- `sgdma` = configBarAddr + dir*BLOCK_OFFSET + ch*ENGINE_OFFSET + SGDMA_BLOCK_OFFSET → `XDMA_SGDMA_REGS`

### 7.3 寄存器结构概览

| 寄存器块 | 结构体 (reg.h) | 关键字段 |
|---|---|---|
| IRQ Block (0x2000) | `XDMA_IRQ_REGS` | identifier, userIntEnable(W1S/W1C), channelIntEnable, userIntRequest, channelIntRequest, userIntPending, channelIntPending, userVector[4], channelVector[2] |
| Config Block (0x3000) | `XDMA_CONFIG_REGS` | identifier, busDev, pcieMPS, pcieMRRS, systemId, msiEnable, pcieWidth, pcieControl, userMPS, userMRRS, writeFlushTimeout |
| Engine Regs | `XDMA_ENGINE_REGS` | identifier, control/controlW1S/W1C, status/statusRC, completedDescCount, alignments, pollModeWbLo/Hi, intEnableMask/W1S/W1C, perfCtrl, perfCyc/Dat/Pnd |
| SGDMA Regs | `XDMA_SGDMA_REGS` | identifier, firstDescLo/Hi, firstDescAdj, descCredits |
| SGDMA Common (0x6000) | `XDMA_SGDMA_COMMON_REGS` | identifier, control, controlW1S/W1C, creditModeEnable/W1S/W1C |

### 7.4 PCI 配置空间读取工具（pcie_common.h）

| 函数 | 用途 |
|---|---|
| `FindCapability(pciBus, pciHeader, capID)` | 遍历 capability 链表，返回指定 cap 的偏移 |
| `GetNumMsiVectors(device, &num)` | 读 MSI cap messageControl，返回 `(1<<((mc&0x70)>>4))` 个 MSI 向量数 |
| `GetNumMsixVectors(device, &num)` | 读 MSI-X cap messageControl，返回 `(mc&0x07FF)+1` 个 MSI-X 向量数 |
| `GetLineInterruptPin(device, &pin)` | 读 `pciHeader.u.type0.InterruptPin`（1=A,2=B,3=C,4=D） |

均通过 `WdfFdoQueryForInterface(GUID_BUS_INTERFACE_STANDARD)` 拿 `BUS_INTERFACE_STANDARD`，再调用 `pciBus.GetBusData(..., PCI_WHICHSPACE_CONFIG, ...)`。

---

## 8. PCIe 资源流

```
[BIO枚举]   PCI 配置空间分配 BAR 物理地址
                │
                v
[PnP Manager] ResourcesRaw / ResourcesTranslated (WDFCMRESLIST)
                │
                v
[EvtDevicePrepareHardware]
        │
        v
   XDMA_DeviceOpen(device, xdma, ..., ResourcesRaw, ResourcesTranslated)
        │
        ├──► MapBARs(xdma, ResourcesTranslated)                    ← device.c:194
        │       │
        │       ├──► GetRealBarIndices(xdma->wdfDevice, realBarIndices, &numRealBars)
        │       │   读 PCI 配置空间 BaseAddresses[0..5]
        │       │   恢复 BAR 真实物理编号
        │       │
        │       └── for each CmResourceTypeMemory resource:
        │           realIdx = realBarIndices[memResIdx++]
        │           xdma->bar[realIdx]       = MmMapIoSpace(Start, Length, MmNonCached)
        │           xdma->barLength[realIdx] = Length
        │           xdma->numBars++
        │
        ├──► IdentifyBars(xdma)                                ← device.c:278
        │       FindConfigBAR → IsConfigBAR (比对 XDMA_ID)
        │       确定 configBarIdx / userBarIdx / bypassBarIdx
        │
        ├──► GetRegisterModules(xdma)                          ← device.c:307
        │       configRegs   = bar[configBarIdx] + CONFIG_BLOCK_OFFSET
        │       interruptRegs = bar[configBarIdx] + IRQ_BLOCK_OFFSET
        │       sgdmaRegs    = bar[configBarIdx] + SGDMA_COMMON_BLOCK_OFFSET
        │
        └──► SetupInterrupts(..., ResourcesRaw, ResourcesTranslated)  ← interrupt.c:590
                使用 ResourcesRaw/Translated 中的 CmResourceTypeInterrupt
                分别走 MSIX / MSI / Line 路径
                写 interruptRegs->userVector/channelVector
```

**关键概念**：
- `ResourcesRaw`：BIOS 分配的原始总线地址
- `ResourcesTranslated`：系统翻译后的物理地址（供 `MmMapIoSpace` 用）
- 两者一一对应，索引一致，`Length` 相同
- `barLength` 字段为 `ULONG`（32 位），>4GB 的 BAR 会走 `CmResourceTypeMemoryLarge`

---

## 9. IO 请求处理流（用户态 → 驱动）

### 9.1 文件节点（DevNode）映射

用户态用 `CreateFile(L"\\\\.\\XDMA<dev>\\h2c_0")` 等打开不同设备节点，`EvtDeviceFileCreate` 通过 `GetDevNodeType` 查 `FileNameLUT`（file_io.c:97）确定类型：

| 文件名 | DEVNODE_TYPE | FILE_CONTEXT 处理 |
|---|---|---|
| `\user` | USER | `u.bar = bar[userBarIdx]` |
| `\control` | CONTROL | `u.bar = bar[configBarIdx]` |
| `\bypass` | BYPASS | `u.bar = bar[bypassBarIdx]` |
| `\h2c_0..3` | H2C | `u.engine = &engines[ch][H2C]`, `queue = engineQueue[H2C][ch]` |
| `\c2h_0..3` | C2H | `u.engine = &engines[ch][C2H]`, `queue = engineQueue[C2H][ch]` (ST 还调 `EngineRingSetup`) |
| `\event_0..15` | EVENTS | `u.event = &userEvents[idx]` |

### 9.2 IO 请求分发图

```
ReadFile/WriteFile/DeviceIoControl
            │
            v
  [I/O Manager] ── IRP ──> [WDFDEVICE]
            │
            ├── EvtDeviceIoInCallerContext (file_io.c:732)  ← caller 线程上下文
            │       │
            │       ├── if IOCTL_MAP_BAR:
            │       │   IoctlMapBar(file, request, barIndex, xdma)  # file_io.c:530
            │       │   ├── IoAllocateMdl(xdma->bar[barIndex], barLength[barIndex], ...)
            │       │   ├── MmBuildMdlForNonPagedPool(file->mdl)
            │       │   ├── virtAddr = MmMapLockedPagesSpecifyCache(mdl, UserMode, MmNonCached, ...)
            │       │   ├── file->virtAddress = virtAddr
            │       │   └── xbar_info->mappedAddress/barLength = virtAddr/barLength
            │       │   完成 request，返回
            │       │
            │       └── else: WdfDeviceEnqueueRequest(device, request)  ← 进默认队列
            │
            v
  [Default Queue (Parallel)] 
            │
            ├── EvtIoRead (file_io.c:358)
            │       │
            │       ├── USER/CONTROL/BYPASS: ReadBarToRequest(request, file->u.bar)  # file_io.c:272
            │       │       WdfRequestRetrieveOutputMemory
            │       │       READ_REGISTER_BUFFER_ULONG/USHORT/UCHAR  ← 寄存器读
            │       │       WdfRequestCompleteWithInformation
            │       │
            │       ├── EVENTS: EvtReadUserEvent(request, length)    # file_io.c:927
            │       │       KeWaitForSingleObject(event, 3 秒超时)
            │       │       WdfMemoryCopyFromBuffer(outputMem, &eventValue)
            │       │
            │       └── C2H: WdfRequestForwardToIoQueue(request, file->queue)  → 引擎队列
            │
            ├── EvtIoWrite (file_io.c:408)
            │       │
            │       ├── USER/CONTROL/BYPASS: WriteBarFromRequest(request, file->u.bar)  # file_io.c:314
            │       │       WdfRequestRetrieveInputMemory
            │       │       WRITE_REGISTER_BUFFER_ULONG/USHORT/UCHAR  ← 寄存器写
            │       │       WdfRequestCompleteWithInformation
            │       │
            │       └── H2C: WdfRequestForwardToIoQueue(request, file->queue) → 引擎队列
            │
            └── EvtIoDeviceControl (file_io.c:774)
                    │
                    ├── USER/CONTROL/BYPASS: IoctlBar(file, request, IoControlCode, barIndex, xdma)
                    │       └── IOCTL_WRITE_KEYHOLE_REGISTER → IoctlKeyholeWriteRegister  # file_io.c:605
                    │           for (size = kholeData->size; totalSize > 0; totalSize--):
                    │               WRITE_REGISTER_ULONG(writeAddr + offset, *ptrAddr++)
                    │
                    └── H2C/C2H: IoctlChannel(file, request, IoControlCode)
                            ├── IOCTL_XDMA_PERF_START  → EngineStartPerf(queue->engine)      # dma_engine.c:1334
                            ├── IOCTL_XDMA_PERF_GET    → IoctlGetPerf → EngineGetPerf         # dma_engine.c:1342
                            ├── IOCTL_XDMA_ADDRMODE_GET → IoctlGetAddrMode
                            └── IOCTL_XDMA_ADDRMODE_SET → IoctlSetAddrMode
                                engine->regs->controlW1S/W1C = XDMA_CTRL_NON_INCR_ADDR

  [Engine Queue (Sequential)]
            │
            ├── (H2C) EvtIoWriteDma (file_io.c:816)              ← H2C 队列
            │       WdfDmaTransactionInitializeUsingRequest(dmaTransaction, Request,
            │                                           XDMA_EngineProgramDma,
            │                                           WdfDmaDirectionWriteToDevice)
            │       WdfDmaTransactionExecute(dmaTransaction, engine)
            │
            ├── (C2H, MM) EvtIoReadDma (file_io.c:853)          ← C2H 队列（MM 类型）
            │       WdfDmaTransactionInitializeUsingRequest(dmaTransaction, Request,
            │                                           XDMA_EngineProgramDma,
            │                                           WdfDmaDirectionReadFromDevice)
            │       WdfDmaTransactionExecute(dmaTransaction, engine)
            │
            └── (C2H, ST) EvtIoReadEngineRing (file_io.c:890)   ← C2H 队列（ST 类型）
                    WdfRequestRetrieveOutputMemory
                    EngineRingCopyBytesToMemory(engine, outputMem, length, 10s timeout, &numBytes)  # dma_engine.c:1123
                    WdfRequestCompleteWithInformation(request, status, numBytes)
```

---

## 10. DMA 数据流

### 10.1 H2C / C2H Scatter-Gather 传输（MM 引擎）

```
EvtIoWriteDma / EvtIoReadDma
        │
        v
WdfDmaTransactionInitializeUsingRequest(... XDMA_EngineProgramDma ...)
        │
        v
WdfDmaTransactionExecute(...)
        │
        v
[WDF] 调用 XDMA_EngineProgramDma(Transaction, Device, engine, Direction, SgList)  ← dma_engine.c:621
        │
        ├── 从 SgList 取每个 scatter-gather element
        ├── 填充 descriptor[i]：
        │   ├── control = XDMA_DESC_MAGIC
        │   ├── numBytes = SgList->Elements[i].Length
        │   ├── if H2C: src = host 物理 / dst = deviceOffset
        │   ├── if C2H: src = deviceOffset / dst = host 物理
        │   ├── nextLo/Hi = 下一个 descriptor 的总线地址
        │   └── 最后一个 descriptor.control |= STOP_BIT | COMPLETED_BIT (ST 还加 EOP_BIT)
        │
        ├── OptimizeDescriptors(engine, descriptor, numDesc)  ← dma_engine.c:331
        │   设置 firstDescAdj / 每个 desc 的 nextAdj，优化 PCIe 读
        │   受 MRRS / 4K 边界 / 描述符总数限制
        │
        ├── markReqPending(engine)
        ├── EngineStart(engine)                                ← dma_engine.c:844
        │       engine->regs->controlW1S = XDMA_CTRL_RUN_BIT
        │
        ├── if poll: EnginePollTransfer(engine, 10s)           ← dma_engine.c:1252
        │       轮询 pollWbBuffer->completedDescCount
        │       完成后调 EngineProcessTransfer
        │
        └── else (中断模式): KeReleaseSemaphore(&engine->semaphore, ...)
                唤醒 xdmaCompletionThread 线程等待完成

[中断或 poll 完成后]
        │
        v
engine->work(engine)  =  EngineProcessTransfer(engine)        ← dma_engine.c:184
        │
        ├── WdfDmaTransactionGetRequest(dmaTransaction)
        ├── EngineStatus(engine, clear=TRUE)                   # 读并清状态
        ├── EngineStop(engine)
        ├── 清空 descBuffer / pollWbBuffer
        ├── unmarkReqPending + EngineEnableInterrupt(engine)   # 重新使能中断
        │
        ├── if status == XDMA_ENGINE_STOPPED_OK:
        │       WdfDmaTransactionDmaCompleted(dmaTransaction, ...)
        │       WdfDmaTransactionGetBytesTransferred → bytesTransferred
        │       KeSetEvent(&engine->completionWaitSignal)      # 唤醒等待线程
        │       WdfDmaTransactionRelease(dmaTransaction)
        │       WdfRequestCompleteWithInformation(request, status, bytesTransferred)
        │
        └── else (error/busy):
                WdfDmaTransactionDmaCompletedFinal(dmaTransaction, 0, ...)
                WdfDmaTransactionRelease(...)
                WdfRequestComplete(request, STATUS_INTERNAL_ERROR)
```

### 10.2 Streaming DMA Ring Buffer（ST 引擎）

```
EvtDeviceFileCreate: 对 C2H ST 引擎调 EngineRingSetup(engine)   ← dma_engine.c:1109
        │
        ├── ring.head = 0, ring.tail = 0
        └── EngineRingProgramDma(engine)                        ← dma_engine.c:1024
                │
                ├── for i in 0..258:
                │   descriptor[i].control = MAGIC | EOP | COMPLETED
                │   descriptor[i].numBytes = PAGE_SIZE (4KB)
                │   descriptor[i].srcAddrLo/Hi = resultBufferLA   ← 硬件覆写
                │   descriptor[i].dstAddrLo/Hi = ring.xmdl[i].dmaAdrr
                │   descriptor[i].nextLo/Hi = 下一个 desc 地址
                │
                ├── 最后一个 desc 指回第一个 desc（环形）
                ├── OptimizeDescriptors(...)
                ├── engine->sgdma->descCredits = 257            ← 初始 credit
                └── EngineStart(engine)

[用户读请求]
        │
        v
EvtIoReadEngineRing(queue, request, length)                    ← file_io.c:890
        │
        └── EngineRingCopyBytesToMemory(engine, outputMem, length, 10s timeout, &bytesRead)
                ← dma_engine.c:1123
                │
                ├── if poll: EnginePollRing(engine, timeout)   # 轮询完成
                ├── else: KeWaitForSingleObject(ring.completionSignal, timeout)
                │
                ├── while (head != tail) && numBytesRemaining:
                │   ├── rxBufferVa = ring.xmdl[head].virtAddr
                │   ├── numBytesReceived = results[head].length
                │   ├── WdfMemoryCopyFromBuffer(outputMem, offset, rxBufferVa, numBytesReceived)
                │   ├── results[head].length = 0
                │   └── EngineRingAdvance(&head)
                │
                └── engine->sgdma->descCredits = numDescProcessed  ← 归还 credit

[硬件完成一段传输后触发 IRQ]
        │
        v
EngineProcessRing(engine)                                       ← dma_engine.c:969
        engine->work = EngineProcessRing (在 EngineCreate 时设置)
        │
        ├── EngineStatus(engine, clear=TRUE)
        ├── for (; results[tail].status; EngineRingAdvance(&tail)):
        │   if status & EOP_BIT: eopCount++
        │   results[tail].status = 0
        └── if eopCount > 0:
                KeSetEvent(&ring.completionSignal)  ← 唤醒读请求
```

### 10.3 Poll 模式线程

```
initThread → xdmaCompletionThread(engine)                      ← dma_engine.c:390
        loop:
            KeWaitForSingleObject(&engine->semaphore, ...)     # 等待 ProgramDma 信号
            if engine->terminate: PsTerminateSystemThread
            if isReqPending(engine):
                EngineWaitForCompletion(engine, 10s timeout)    # dma_engine.c:1222
                    KeWaitForSingleObject(&engine->completionWaitSignal, ...)
                    if timeout: 清状态 + EngineStop
```

---

## 11. 中断处理流

### 11.1 三种中断模式建立

```
SetupInterrupts (interrupt.c:590)
        │
        ├── CountChannels(xdma, &h2c, &c2h)
        ├── GetNumMsixVectors / GetNumMsiVectors / CountInterruptResources
        ├── totRequiredIrqs = userMax + h2cChannelMax + c2hChannelMax
        │
        ├── if numMsixVectors >= totRequiredIrqs:           # MSI-X
        │       SetupMsixInterrupts (interrupt.c:274)
        │           for i in 0..numResources:
        │               if interruptCount < userMax:
        │                   SetupUserInterrupt(xdma, interruptCount, raw, translated)
        │                       WdfInterruptCreate(EvtUserInterruptIsr/Dpc, ...)
        │                       irqContext->eventId = index
        │                       irqContext->regs = interruptRegs
        │               else:
        │                   SetupChannelInterrupt(xdma, interruptCount - userMax, raw, translated)
        │                       WdfInterruptCreate(EvtChannelInterruptIsr/Dpc, ...)
        │                       irqContext->engine = NULL  (EngineConfigureInterrupt 中绑定)
        │           programInterrupts(xdma, h2c, c2h)        # 写 userVector/channelVector
        │
        ├── elif numMsiVectors >= totRequiredIrqs:          # Multi-MSI
        │       SetupMultiMsiInterrupts (interrupt.c:312)
        │           for n in 0..userMax: SetupUserInterrupt(...)
        │           for n in 0..(h2c+c2h): SetupChannelInterrupt(...)
        │           programInterrupts(...)
        │
        └── elif numIrqResources != 0:                       # 单 Line 中断
                SetupSingleInterrupt (interrupt.c:180)
                    SetupDeviceInterrupt(xdma, raw, translated)
                        WdfInterruptCreate(EvtInterruptIsr/Dpc, ...) → xdma->lineInterrupt
                    if Line: GetLineInterruptPin(&vectorValue); vectorValue--
                    写 userVector/channelVector 全部用同一 vectorValue
```

### 11.2 中断 ISR/DPC 处理

**Line 中断模式**（单 IRQ 服务所有事件）：

```
EvtInterruptIsr(Interrupt, MessageID)                         ← interrupt.c:399
        │
        ├── chIrq = regs->channelIntRequest
        │   if chIrq:
        │       irq->channelIrqPending |= chIrq
        │       regs->channelIntEnableW1C = chIrq          ← 屏蔽已触发的
        │
        ├── userIrq = regs->userIntRequest
        │   if userIrq:
        │       irq->userIrqPending |= userIrq
        │       regs->userIntEnableW1C = userIrq
        │
        ├── if !chIrq && !userIrq: return FALSE (spurious)
        └── WdfInterruptQueueDpcForIsr(Interrupt)           ← 调度 DPC

EvtInterruptDpc(interrupt, device)                            ← interrupt.c:442
        │
        ├── for dir, ch in engines:
        │       if (channelIrqPending & engine->irqBitMask):
        │           engine->work(engine)  ← EngineProcessTransfer / Ring
        │
        ├── for i in 0..16:
        │       if (userIrqPending & BIT_N(i)) && userEvents[i].work:
        │           userEvents[i].work(i, userEvents[i].userData)
        │           ↑ HandleUserEvent(eventId, &ctx->eventSignals[i])
        │             KePulseEvent(event, IO_NO_INCREMENT, FALSE)  ← file_io.c:987
        │
        └── WdfInterruptAcquireLock/ReleaseLock:
                regs->channelIntEnableW1S = channelIrqPending; channelIrqPending = 0
                regs->userIntEnableW1S = userIrqPending;    userIrqPending = 0
```

**MSI/MSI-X 模式**（每个 engine/event 独立 IRQ）：

```
EvtChannelInterruptIsr(Interrupt, MessageID)                  ← interrupt.c:512
        EngineDisableInterrupt(irq->engine)
        return WdfInterruptQueueDpcForIsr(Interrupt)

EvtChannelInterruptDpc(interrupt, device)                     ← interrupt.c:527
        irq->engine->work(irq->engine)    ← EngineProcessTransfer/Ring

EvtUserInterruptIsr(Interrupt, MessageID)                     ← interrupt.c:557
        regs->userIntEnableW1C = BIT_N(MessageID)          ← 禁用该 event
        return WdfInterruptQueueDpcForIsr(Interrupt)

EvtUserInterruptDpc(interrupt, device)                        ← interrupt.c:567
        userEvent = &xdma->userEvents[irq->eventId]
        if userEvent->work:
            userEvent->work(eventId, userData)    ← HandleUserEvent
        WdfInterruptAcquireLock/ReleaseLock:
            regs->userIntEnableW1S = BIT_N(eventId)        ← 重新使能
```

---

## 12. WPP 跟踪

`trace.h` 定义了控制 GUID `{7dd02079-3c3f-42c5-9384-c210c7cc490a}`，6 个 trace bit：

| Flag | 用途 |
|---|---|
| `DBG_INIT` | 初始化路径 |
| `DBG_IRQ` | 中断路径 |
| `DBG_DMA` | DMA 传输路径 |
| `DBG_DESC` | 描述符 dump |
| `DBG_USER` | 用户事件 |
| `DBG_IO` | IO 请求处理 |

Release 构建中 `TraceVerbose/Info/Warning/Error` 全部 stub 为空操作；Debug 构建通过 `traceview.exe` / `logman.exe` 抓取。

---

## 13. 用户接口总览

### 13.1 设备接口 GUID

```
GUID_DEVINTERFACE_XDMA = {74c7e4a9-6d5d-4a70-bc0d-20691dff9e9d}
```

### 13.2 设备节点（文件名）

| 文件名 | 用途 |
|---|---|
| `\user` | 用户 BAR 寄存器读写（AXI-Lite） |
| `\control` | 配置 BAR 寄存器读写 |
| `\bypass` | Bypass BAR 直接描述符提交 |
| `\h2c_0..3` | H2C DMA 通道（host→card） |
| `\c2h_0..3` | C2H DMA 通道（card→host） |
| `\event_0..15` | 16 路用户事件等待 |

### 13.3 IOCTL 码

| IOCTL | 值 | 用途 |
|---|---|---|
| `IOCTL_XDMA_GET_VERSION` | 0x0 | 获取驱动版本 |
| `IOCTL_XDMA_PERF_START` | 0x1 | 启动引擎性能计数 |
| `IOCTL_XDMA_PERF_STOP` | 0x2 | 停止性能计数 |
| `IOCTL_XDMA_PERF_GET` | 0x3 | 读取性能计数（返回 `XDMA_PERF_DATA`） |
| `IOCTL_XDMA_ADDRMODE_GET` | 0x4 | 获取地址模式（递增/固定） |
| `IOCTL_XDMA_ADDRMODE_SET` | 0x5 | 设置地址模式 |
| `IOCTL_WRITE_KEYHOLE_REGISTER` | 0x6 | 用户 BAR 钥匙孔写入（`XDMA_KEYHOLE_DATA`） |
| `IOCTL_MAP_BAR` | 0x7 | 把 BAR 映射到用户态（`XDMA_BAR_INFO`） |

### 13.4 用户结构体

```c
typedef struct { UINT64 clockCycleCount, dataCycleCount, pendingCount; } XDMA_PERF_DATA;
typedef struct { PULONG ptrAddr; ULONG size; ULONG offset; }            XDMA_KEYHOLE_DATA;
typedef struct { PVOID mappedAddress; ULONG barLength; }                XDMA_BAR_INFO;
```

---

## 14. 关键函数索引

### 14.1 driver.c

| 函数 | 行号 | 类型 | 说明 |
|---|---|---|---|
| `DriverEntry` | 130 | export | WDF 驱动入口 |
| `DriverUnload` | 168 | export | 卸载清理 |
| `EvtDeviceAdd` | 178 | WDF cb | 创建 WDFDEVICE、注册回调、创建默认队列 |
| `EvtDeviceCleanup` | 259 | WDF cb | 设备清理 |
| `EvtDevicePrepareHardware` | 266 | WDF cb | 调用 XDMA_DeviceOpen、设置 poll、创建引擎队列、注册用户事件 |
| `EvtDeviceReleaseHardware` | 324 | WDF cb | 调用 XDMA_DeviceClose |
| `EngineCreateQueue` | 339 | static | 为引擎创建顺序队列，按方向绑定 EvtIoWriteDma / EvtIoReadDma / EvtIoReadEngineRing |
| `GetPollModeParameter` | 100 | static | 读注册表 POLL_MODE |

### 14.2 device.c

| 函数 | 行号 | 类型 | 说明 |
|---|---|---|---|
| `XDMA_DeviceOpen` | 316 | export | 设备初始化总入口：MapBARs + IdentifyBars + GetRegisterModules + SetupInterrupts + WdfDmaEnablerCreate + ProbeEngines |
| `XDMA_DeviceClose` | 329 | export | 设备关闭：清 IRQ 向量 + closeEngines + MmUnmapIoSpace |
| `DeviceDefaultInitialize` | 116 | static | 清零 XDMA_DEVICE 所有字段 |
| `GetRealBarIndices` | 153 | static | 读 PCI 配置空间恢复真实 BAR 编号 |
| `MapBARs` | 194 | static | 把 CmResourceTypeMemory 资源映射成内核虚拟地址 |
| `IsConfigBAR` | 200 | static | 读 IRQ+CONFIG identifier 判断是否配置 BAR |
| `FindConfigBAR` | 212 | static | 遍历 bar[] 找配置 BAR |
| `IdentifyBars` | 222 | static | 确定 user/config/bypass BAR 索引 |
| `GetRegisterModules` | 222 | static | 计算 config/irq/sgdma 寄存器指针 |
| `GetVersion` | 109 | static | 读 IP 版本 |

### 14.3 dma_engine.c

| 函数 | 行号 | 类型 | 说明 |
|---|---|---|---|
| `ProbeEngines` | 778 | export | 遍历 H2C/C2H 所有通道，调 EngineCreate 创建引擎 |
| `CountChannels` | 759 | export | 通过 EngineExists 统计通道数 |
| `EngineCreate` | 487 | static | 引擎初始化：寄存器绑定、IRQ 配置、描述符缓冲、Ring、线程 |
| `EngineExists` | 382 | static | 读 engineRegs.identifier 判断通道是否存在 |
| `EngineConfigureInterrupt` | 161 | static | 设置 irqBitMask，绑定 IRQ context |
| `EngineGetAlignments` | 571 | static | 读 alignments 寄存器 |
| `EngineCreateDescriptorBuffer` | 132 | static | 分配描述符 WDFCOMMONBUFFER，写入 firstDescLo/Hi |
| `EngineCreateRingBuffer` | 881 | static | ST 引擎分配 results + 258×4KB ring + spinlock + event |
| `EngineCreatePollWriteBackBuffer` | 1200 | static | 分配 poll 写回 buffer，写入 pollModeWbLo/Hi |
| `EngineRingProgramDma` | 1024 | static | 填环形描述符表，启动循环 DMA |
| `XDMA_EngineProgramDma` | 621 | WDF cb | WDF 回调，从 SgList 填充描述符并启动引擎 |
| `EngineProcessTransfer` | 184 | static | MM 引擎完成处理：清状态、调 WdfDmaTransactionDmaCompleted、完成请求 |
| `EngineProcessRing` | 969 | static | ST 引擎完成处理：扫 results[tail]，统计 EOP，唤醒读队列 |
| `EngineStart` | 844 | export | controlW1S = RUN_BIT |
| `EngineStop` | 850 | export | controlW1C = RUN_BIT |
| `EngineEnableInterrupt` | 856 | export | channelIntEnableW1S = irqBitMask |
| `EngineDisableInterrupt` | 866 | export | channelIntEnableW1C = irqBitMask |
| `EngineStartPerf` | 1334 | export | perfCtrl = CLEAR → AUTO\|RUN |
| `EngineGetPerf` | 1342 | export | 读 perfCyc/Dat/Pnd 拼成 64-bit |
| `EngineRingSetup` | 1109 | export | 重置 ring head/tail + EngineRingProgramDma |
| `EngineRingTeardown` | 1116 | export | EngineStop + 清 results + 重置 head/tail |
| `EngineRingCopyBytesToMemory` | 1123 | export | 把 ring 数据拷到 WDFMEMORY |
| `EnginePollRing` | 1297 | static | 轮询 pollWb，调 EngineProcessRing |
| `EnginePollTransfer` | 1252 | static | 轮询 pollWb，调 EngineProcessTransfer |
| `EngineWaitForCompletion` | 1222 | static | 等 completionWaitSignal，超时清理 |
| `xdmaCompletionThread` | 390 | static | 系统线程：等 semaphore → EngineWaitForCompletion |
| `initThread` | 434 | static | 创建系统线程 |
| `terminateThread` | 466 | static | 终止线程 |
| `OptimizeDescriptors` | 331 | static | 优化 PCIe 读：设 firstDescAdj / nextAdj |
| `DescriptorIsAligned` | 301 | static | 检查 src/dst/length 对齐 |
| `closeEngines` | 829 | export | 终止所有引擎线程 |
| `XDMA_EngineSetPollMode` | 1356 | export | 配置引擎为 poll 或中断模式 |

### 14.4 interrupt.c

| 函数 | 行号 | 类型 | 说明 |
|---|---|---|---|
| `SetupInterrupts` | 590 | export | 中断建立总入口：探测 MSI-X/MSI/Line，分发 |
| `SetupUserInterrupt` | 105 | static | 创建用户 IRQ WDFINTERRUPT |
| `SetupChannelInterrupt` | 132 | static | 创建通道 IRQ WDFINTERRUPT |
| `SetupDeviceInterrupt` | 157 | static | 创建 Line IRQ WDFINTERRUPT |
| `SetupMsixInterrupts` | 274 | static | MSI-X 模式建立 |
| `SetupMultiMsiInterrupts` | 312 | static | Multi-MSI 模式建立 |
| `SetupSingleInterrupt` | 180 | static | Line 模式建立 |
| `programInterrupts` | 234 | static | 写 userVector/channelVector |
| `CountInterruptResources` | 356 | static | 统计 CmResourceTypeInterrupt 数量 |
| `BuildVectorReg` | 96 | static | 把 4 个 5-bit vector 拼成一个 32-bit |
| `EvtInterruptIsr/Dpc` | 399/442 | WDF cb | Line 中断 ISR/DPC（单 IRQ 服务全部） |
| `EvtChannelInterruptIsr/Dpc` | 512/527 | WDF cb | 通道 IRQ ISR/DPC |
| `EvtUserInterruptIsr/Dpc` | 557/567 | WDF cb | 用户 IRQ ISR/DPC |
| `EvtInterruptEnable/Disable` | 377/388 | WDF cb | Line IRQ enable/disable，写 channelIntEnableW1S/W1C |
| `EvtChannelInterruptEnable/Disable` | 494/503 | WDF cb | 通道 IRQ enable/disable，调 EngineEnableInterrupt/DisableInterrupt |
| `EvtUserInterruptEnable/Disable` | 537/547 | WDF cb | 用户 IRQ enable/disable，写 userIntEnableW1S/W1C |
| `XDMA_UserIsrRegister` | 668 | export | 注册用户事件回调 |
| `XDMA_UserIsrEnable/Disable` | 680/691 | export | 使能/禁用用户事件中断 |

### 14.5 file_io.c

| 函数 | 行号 | 类型 | 说明 |
|---|---|---|---|
| `EvtDeviceFileCreate` | 141 | WDF cb | 文件创建：查 FileNameLUT，绑定 bar/engine/event |
| `EvtFileClose` | 226 | WDF cb | 文件关闭 |
| `EvtFileCleanup` | 231 | WDF cb | 文件清理：ST 引擎 teardown、解除 BAR 用户态映射 |
| `GetDevNodeType` | 127 | static | 文件名 → DEVNODE_TYPE |
| `EvtIoRead` | 358 | WDF cb | 默认队列读：BAR 读 / Event 等待 / 转发到引擎队列 |
| `EvtIoWrite` | 408 | WDF cb | 默认队列写：BAR 写 / 转发到引擎队列 |
| `EvtIoDeviceControl` | 774 | WDF cb | IOCTL 分发：IoctlBar / IoctlChannel |
| `EvtDeviceIoInCallerContext` | 732 | WDF cb | caller 上下文：处理 IOCTL_MAP_BAR |
| `EvtIoWriteDma` | 816 | WDF cb | H2C 引擎队列：初始化 DMA 事务 |
| `EvtIoReadDma` | 853 | WDF cb | C2H MM 引擎队列：初始化 DMA 事务 |
| `EvtIoReadEngineRing` | 890 | WDF cb | C2H ST 引擎队列：从 ring 拷贝数据 |
| `EvtReadUserEvent` | 927 | static | 用户事件等待：KeWaitForSingleObject 3s 超时 |
| `HandleUserEvent` | 987 | static | 用户事件回调：KePulseEvent |
| `ReadBarToRequest` | 272 | static | BAR → WDFMEMORY（READ_REGISTER_BUFFER_*） |
| `WriteBarFromRequest` | 314 | static | WDFMEMORY → BAR（WRITE_REGISTER_BUFFER_*） |
| `ValidateBarParams` | 251 | static | 校验 BAR 索引和 offset+length |
| `IoctlMapBar` | 530 | static | BAR 用户态映射（MmMapLockedPagesSpecifyCache） |
| `IoctlKeyholeWriteRegister` | 605 | static | 钥匙孔重复写 |
| `IoctlBar` | 660 | static | BAR IOCTL 分发 |
| `IoctlChannel` | 681 | static | 通道 IOCTL 分发 |
| `IoctlGetPerf` | 450 | static | 获取性能计数 |
| `IoctlGetAddrMode` | 474 | static | 获取地址模式 |
| `IoctlSetAddrMode` | 498 | static | 设置地址模式 |
| `EvtCancelReadUserEvent` | 915 | WDF cb | 用户事件请求取消 |

---

## 15. 配置与构建

### 15.1 INF（simple_xdma.inf）

- `Class = System`，`ClassGuid = {4d36e97d-e325-11ce-bfc1-08002be10318}`
- `ServiceType = 1` (SERVICE_KERNEL_DRIVER)
- `StartType = 3` (SERVICE_DEMAND_START)
- `ErrorControl = 1` (SERVICE_ERROR_NORMAL)
- `PnpLockdown = 1`（启用 PnP 文件锁定，文件从 %13% 加载）
- `KmdfLibraryVersion = $KMDFVERSION$`（构建期 stampinf 替换）

### 15.2 注册表参数

`HKLM\SYSTEM\CurrentControlSet\Services\simple_xdma\Parameters\POLL_MODE`（ULONG）
- 0 = 中断模式（默认）
- 非 0 = poll 模式

### 15.3 Pageable 代码

`#pragma alloc_text(PAGE, ...)` 标记的可分页函数：
- `DriverEntry`（INIT 段）
- `DriverUnload`、`EvtDeviceAdd`、`EvtDevicePrepareHardware`、`EvtDeviceReleaseHardware`、`EngineCreateQueue`
- `ProbeEngines`

### 15.4 编译告警屏蔽

```c
#pragma warning(disable: 30029)  // MmMapIoSpace → MmMapIoSpaceEx (Win10 only)
#pragma warning(disable: 30030)  // MdlMappingNoExecute (Win8+ only)
```

---

## 16. 关键常量速查

| 常量 | 值 | 出处 | 含义 |
|---|---|---|---|
| `XDMA_MAX_NUM_BARS` | 3 | device.h:72 | BAR 数组大小 |
| `XDMA_MAX_NUM_CHANNELS` | 4 | dma_engine.h:73 | 每方向最大通道数 |
| `XDMA_NUM_DIRECTIONS` | 2 | dma_engine.h:74 | H2C + C2H |
| `XDMA_MAX_CHAN_IRQ` | 8 | dma_engine.h:75 | 4×2 通道 IRQ 数 |
| `XDMA_MAX_USER_IRQ` | 16 | interrupt.h:69 | 用户 IRQ 数 |
| `XDMA_RING_NUM_BLOCKS` | 258 | dma_engine.h:76 | Ring 块数 |
| `XDMA_RING_BLOCK_SIZE` | 4096 | dma_engine.h:77 | Ring 块大小（PAGE_SIZE） |
| `XDMA_MAX_TRANSFER_SIZE` | 8MB | dma_engine.h:78 | 单次传输最大字节 |
| `BLOCK_OFFSET` | 0x1000 | reg.h:85 | 引擎方向间偏移 |
| `ENGINE_OFFSET` | 0x100 | reg.h:90 | 通道间偏移 |
| `IRQ_BLOCK_OFFSET` | 0x2000 | reg.h:86 | IRQ 寄存器块 |
| `CONFIG_BLOCK_OFFSET` | 0x3000 | reg.h:87 | Config 寄存器块 |
| `SGDMA_BLOCK_OFFSET` | 0x4000 | reg.h:88 | SGDMA 寄存器块 |
| `SGDMA_COMMON_BLOCK_OFFSET` | 0x6000 | reg.h:89 | SGDMA 公共寄存器块 |
| `XDMA_ID_MASK` | 0xFFF00000 | reg.h:77 | identifier 掩码 |
| `XDMA_ID` | 0x1FC00000 | reg.h:78 | XDMA block 标识 |
| `XDMA_DESC_MAGIC` | 0xAD4B0000 | dma_engine.c:77 | 描述符 magic |
| `XDMA_CTRL_RUN_BIT` | BIT(0) | reg.h:93 | 启动引擎 |
| `XDMA_CTRL_POLL_MODE` | BIT(26) | reg.h:104 | poll 模式 |
| `XDMA_CTRL_NON_INCR_ADDR` | BIT(25) | reg.h:103 | 非递增地址 |
| `XDMA_DESC_STOP_BIT` | BIT(0) | reg.h:130 | 描述符停止位 |
| `XDMA_DESC_COMPLETED_BIT` | BIT(1) | reg.h:131 | 描述符完成位 |
| `XDMA_DESC_EOP_BIT` | BIT(4) | reg.h:132 | 包结束位 |

---

## 17. 调试入口

| 关注点 | 推荐位置 |
|---|---|
| BAR 映射是否正确 | `MapBARs` 的 `TraceInfo("MM BAR %d ...")` |
| 配置 BAR 识别 | `FindConfigBAR` 的 `TraceInfo("config BAR is %u")` |
| 引擎探测 | `ProbeEngines` 的 `TraceInfo("engine created (AXI-%s)")` |
| 中断模式选择 | `SetupInterrupts` 的 `TraceVerbose("numIrqResources=... MSIX=... MSI=...")` |
| 中断触发 | `EvtInterruptIsr` 的 `TraceInfo("irq messageId = %u")` |
| DMA 传输完成 | `EngineProcessTransfer` 的 `TraceInfo("transaction%scomplete, bytesTransferred=%llu")` |
| 描述符 dump | `DumpDescriptor`（仅 DBG） |
| 用户事件 | `HandleUserEvent` 的 `TraceInfo("event_%u signaling completion")` |
| IO 请求分发 | `EvtIoRead/Write` 的 `TraceVerbose("devNodeType %d")` |

---

*文档生成日期：2026-08-22*
*基于驱动源码版本：simple_xdma（Xilinx 2019 修订）*
