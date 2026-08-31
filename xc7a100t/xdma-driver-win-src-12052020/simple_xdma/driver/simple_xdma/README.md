## 项目开发步骤

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



## NTSTATUS 常用数值

`NTSTATUS` 是 32-bit 有符号整数，`STATUS_SEVERITY_SUCCESS(0)` 即 `NT_SUCCESS()` 为 `>= 0`，错误为 `< 0`。以下是你在 XDMA 驱动开发中最常用的值：

------

### 一、成功（NT_SUCCESS 返回 TRUE）

| 值                                | 数值         | 含义                                   |
| --------------------------------- | ------------ | -------------------------------------- |
| `STATUS_SUCCESS`                  | `0x00000000` | 操作成功                               |
| `STATUS_PENDING`                  | `0x00000103` | 异步操作挂起中                         |
| `STATUS_MORE_PROCESSING_REQUIRED` | `0xC0000016` | 注意：这个是**错误码**，虽然名字像成功 |

------

### 二、通用错误（你的代码中已大量使用）

| 值                              | 含义         | 使用场景                                                 |
| ------------------------------- | ------------ | -------------------------------------------------------- |
| `STATUS_UNSUCCESSFUL`           | `0xC0000001` | 通用失败，不知道怎么分类时用                             |
| `STATUS_INTERNAL_ERROR`         |              | 驱动内部逻辑错误                                         |
| `STATUS_INVALID_PARAMETER`      | `0xC000000D` | 参数无效（NULL 指针、越界等）                            |
| `STATUS_INSUFFICIENT_RESOURCES` | `0xC000009A` | 内存不足、资源耗尽                                       |
| `STATUS_TIMEOUT`                | `0x00000102` | 看门狗超时（注意：这是**成功码**，`NT_SUCCESS` 为 TRUE） |
| `STATUS_CANCELLED`              | `0xC0000120` | 操作被取消                                               |

------

### 三、PCI/设备相关（XDMA 驱动高频使用）

| 值                                  | 含义         | 使用场景                            |
| ----------------------------------- | ------------ | ----------------------------------- |
| `STATUS_DEVICE_CONFIGURATION_ERROR` | `0xC0000182` | PCI 配置空间读取失败、BAR 映射失败  |
| `STATUS_DRIVER_INTERNAL_ERROR`      |              | 驱动内部错误（如找不到 config BAR） |
| `STATUS_INVALID_DEVICE_REQUEST`     | `0xC0000010` | PCI 配置空间读不到数据              |
| `STATUS_NOINTERFACE`                |              | MSI/MSI-X capability 不存在         |
| `STATUS_DEVICE_NOT_READY`           | `0xC00000A3` | 硬件未就绪                          |

------

### 四、IO 操作相关

| 值                           | 含义         | 使用场景               |
| ---------------------------- | ------------ | ---------------------- |
| `STATUS_BUFFER_TOO_SMALL`    | `0xC0000023` | 用户态缓冲区小于所需   |
| `STATUS_INVALID_BUFFER_SIZE` |              | 缓冲区大小不合法       |
| `STATUS_END_OF_FILE`         | `0xC0000011` | C2H 读操作到达数据末尾 |
| `STATUS_NOT_SUPPORTED`       | `0xC00000BB` | 不支持的 IOCTL / 操作  |

------

### 五、中断相关

| 值                       | 含义 | 使用场景         |
| ------------------------ | ---- | ---------------- |
| `STATUS_NO_MORE_ENTRIES` |      | MSI-X 向量不够用 |
| `STATUS_NOT_FOUND`       |      | 找不到指定中断   |

------

### 六、你的 XDMA 代码中实际出现的统计

从你的代码库中统计到的 `STATUS_*` 使用频率：

```
STATUS_SUCCESS                   ★★★★★  (最多，所有正常路径返回)
STATUS_INVALID_PARAMETER         ★★★★   (参数校验失败)
STATUS_INTERNAL_ERROR            ★★★    (内部错误)
STATUS_DEVICE_CONFIGURATION_ERROR ★★    (BAR/PCI 配置错误)
STATUS_TIMEOUT                   ★★    (DMA 看门狗超时)
STATUS_UNSUCCESSFUL              ★★    (通用失败)
STATUS_INSUFFICIENT_RESOURCES    ★      (WdfCommonBufferCreate 失败)
STATUS_DRIVER_INTERNAL_ERROR     ★      (FindConfigBAR 失败)
STATUS_INVALID_DEVICE_REQUEST    ★      (PCI 配置读失败)
STATUS_NOINTERFACE               ★      (MSI/MSI-X 不存在)
```

------

### 七、判断宏

![img](https://file+.vscode-resource.vscode-cdn.net/c%3A/Users/Lenovo/.vscode/extensions/marscode.marscode-extension-1.6.36/resource/images/languageIcon/c.svg)

```
// 判断成功
if (NT_SUCCESS(status)) { ... }

// 判断错误
if (!NT_SUCCESS(status)) { ... }
```

**注意陷阱**：`STATUS_TIMEOUT` (`0x00000102`) 和 `STATUS_PENDING` (`0x00000103`) 都是**成功码**（`>= 0`），`NT_SUCCESS()` 对它们返回 TRUE。如果你在超时分支中想当作错误处理，需要明确判断：

```
if (status == STATUS_TIMEOUT) {
    // 超时，虽然不是错误码但需要特殊处理
}
```

------

### 八、用 `%!STATUS!` 打印可读名称

在你的代码中已经大量使用了 WPP 的 `%!STATUS!` 格式说明符：

```
TraceError(DBG_INIT, "WdfDriverCreate failed: %!STATUS!", status);
// 输出示例: "WdfDriverCreate failed: STATUS_INSUFFICIENT_RESOURCES"
```

`%!STATUS!` 会自动把 `NTSTATUS` 数值转换为可读的字符串名称，非常方便调试。