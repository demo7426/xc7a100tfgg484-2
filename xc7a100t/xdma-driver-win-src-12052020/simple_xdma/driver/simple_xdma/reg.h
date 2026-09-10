/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	reg.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.09.06
描  述: 寄存器文件
备  注:
修改记录:

  1.  日期: 2026.09.06
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#ifndef __REG_H__
#define __REG_H__

#define CONFIG_BAR_INDEX            (2)

#define IRQ_BLOCK_REGISTERS        (0x2000)

// ===== 新增内容 =====

// 块偏移量
#define BLOCK_OFFSET            (0x1000)
#define SGDMA_BLOCK_OFFSET      (4 * BLOCK_OFFSET)   // 0x4000
#define SGDMA_COMMON_BLOCK_OFFSET (6 * BLOCK_OFFSET) // 0x6000
#define ENGINE_OFFSET           (0x100)

// XDMA 标识
#define XDMA_ID_MASK            (0xFFF00000UL)
#define XDMA_ID                 (0x1FC00000UL)
#define XDMA_ID_ST_BIT          (1 << 15)

// 控制寄存器位
#define XDMA_CTRL_RUN_BIT                   (1 << 0)
#define XDMA_CTRL_IE_DESC_STOPPED           (1 << 1)
#define XDMA_CTRL_IE_DESC_COMPLETED         (1 << 2)
#define XDMA_CTRL_IE_ALIGNMENT_MISMATCH     (1 << 3)
#define XDMA_CTRL_IE_MAGIC_STOPPED          (1 << 4)
#define XDMA_CTRL_IE_INVALID_LENGTH         (1 << 5)
#define XDMA_CTRL_IE_IDLE_STOPPED           (1 << 6)
#define XDMA_CTRL_IE_READ_ERROR             (0x1f << 9)
#define XDMA_CTRL_IE_WRITE_ERROR            (0x1f << 14)
#define XDMA_CTRL_IE_DESCRIPTOR_ERROR       (0x1f << 19)
#define XDMA_CTRL_NON_INCR_ADDR             (1 << 25)
#define XDMA_CTRL_POLL_MODE                 (1 << 26)
#define XDMA_CTRL_RST                       (1 << 31)

#define XDMA_CTRL_IE_ALL ( \
    XDMA_CTRL_IE_DESC_STOPPED | XDMA_CTRL_IE_DESC_COMPLETED | \
    XDMA_CTRL_IE_ALIGNMENT_MISMATCH | XDMA_CTRL_IE_MAGIC_STOPPED | \
    XDMA_CTRL_IE_INVALID_LENGTH | XDMA_CTRL_IE_READ_ERROR | \
    XDMA_CTRL_IE_WRITE_ERROR | XDMA_CTRL_IE_DESCRIPTOR_ERROR)

// 状态寄存器位
#define XDMA_BUSY_BIT                       (1 << 0)
#define XDMA_DESCRIPTOR_STOPPED_BIT         (1 << 1)
#define XDMA_DESCRIPTOR_COMPLETED_BIT       (1 << 2)
#define XDMA_ALIGN_MISMATCH_BIT             (1 << 3)
#define XDMA_MAGIC_STOPPED_BIT              (1 << 4)
#define XDMA_FETCH_STOPPED_BIT              (1 << 5)
#define XDMA_IDLE_STOPPED_BIT               (1 << 6)
#define XDMA_STAT_READ_ERROR                (0x1fUL * (1 << 9))
#define XDMA_STAT_DESCRIPTOR_ERROR          (0x1fUL * (1 << 19))

// 描述符控制位
#define XDMA_DESC_STOP_BIT                  (1 << 0)
#define XDMA_DESC_COMPLETED_BIT             (1 << 1)
#define XDMA_DESC_EOP_BIT                   (1 << 4)
#define XDMA_DESC_MAGIC                     (0xAD4B0000)

// 最大传输大小
#define XDMA_MAX_TRANSFER_SIZE              (8UL * 1024UL * 1024UL)

#pragma pack(1)

// H2C/C2H 通道寄存器 (H2C: 0x0000, C2H: 0x1000)
typedef struct _XDMA_ENGINE_REGS 
{
    UINT32 identifier;
    UINT32 control;
    UINT32 controlW1S;
    UINT32 controlW1C;
    UINT32 reserved_1[12];
    UINT32 status;
    UINT32 statusRC;
    UINT32 completedDescCount;
    UINT32 alignments;
    UINT32 reserved_2[14];
    UINT32 pollModeWbLo;
    UINT32 pollModeWbHi;
    UINT32 intEnableMask;
    UINT32 intEnableMaskW1S;
    UINT32 intEnableMaskW1C;
    UINT32 reserved_3[9];
    UINT32 perfCtrl;
    UINT32 perfCycLo;
    UINT32 perfCycHi;
    UINT32 perfDatLo;
    UINT32 perfDatHi;
    UINT32 perfPndLo;
    UINT32 perfPndHi;
} XDMA_ENGINE_REGS;

// SGDMA 寄存器 (H2C: 0x4000, C2H: 0x5000)
typedef struct _XDMA_SGDMA_REGS 
{
    UINT32 identifier;
    UINT32 reserved_1[31];
    UINT32 firstDescLo;
    UINT32 firstDescHi;
    UINT32 firstDescAdj;
    UINT32 descCredits;
} XDMA_SGDMA_REGS;

// DMA 描述符 (8×32bit = 32字节)
typedef struct {
    UINT32 control;
    UINT32 numBytes;
    UINT32 srcAddrLo;
    UINT32 srcAddrHi;
    UINT32 dstAddrLo;
    UINT32 dstAddrHi;
    UINT32 nextLo;
    UINT32 nextHi;
} DMA_DESCRIPTOR;

#pragma pack()

#endif // !__REG_H__
