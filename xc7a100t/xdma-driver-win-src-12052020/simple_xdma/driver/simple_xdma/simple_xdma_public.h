/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	simple_xdma_public.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.09.01
描  述: 公共文件
备  注:
修改记录:

  1.  日期: 2026.09.01
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <initguid.h>

#define BAR_MAX_NUM    6       //Bar的最大数量


// 74c7e4a9-6d5d-4a70-bc0d-20691dff9e9d
DEFINE_GUID(GUID_DEVINTERFACE_XDMA,
    0x74c7e4a9, 0x6d5d, 0x4a70, 0xbc, 0x0d, 0x20, 0x69, 0x1d, 0xff, 0x9e, 0x9d);

#define	XDMA_FILE_USER		L"\\user"
#define	XDMA_FILE_CONTROL	L"\\control"
#define XDMA_FILE_BYPASS_0	L"\\bypass_0"
#define XDMA_FILE_BYPASS_1	L"\\bypass_1"
#define XDMA_FILE_BYPASS_2	L"\\bypass_2"
#define XDMA_FILE_BYPASS_3	L"\\bypass_3"

#define	XDMA_FILE_EVENT_0	L"\\event_0"
#define	XDMA_FILE_EVENT_1	L"\\event_1"
#define	XDMA_FILE_EVENT_2	L"\\event_2"
#define	XDMA_FILE_EVENT_3	L"\\event_3"
#define	XDMA_FILE_EVENT_4	L"\\event_4"
#define	XDMA_FILE_EVENT_5	L"\\event_5"
#define	XDMA_FILE_EVENT_6	L"\\event_6"
#define	XDMA_FILE_EVENT_7	L"\\event_7"
#define	XDMA_FILE_EVENT_8	L"\\event_8"
#define	XDMA_FILE_EVENT_9	L"\\event_9"
#define	XDMA_FILE_EVENT_10	L"\\event_10"
#define	XDMA_FILE_EVENT_11	L"\\event_11"
#define	XDMA_FILE_EVENT_12	L"\\event_12"
#define	XDMA_FILE_EVENT_13	L"\\event_13"
#define	XDMA_FILE_EVENT_14	L"\\event_14"
#define	XDMA_FILE_EVENT_15	L"\\event_15"

#define	XDMA_FILE_H2C_0		L"\\h2c_0"
#define	XDMA_FILE_H2C_1		L"\\h2c_1"
#define	XDMA_FILE_H2C_2		L"\\h2c_2"
#define	XDMA_FILE_H2C_3		L"\\h2c_3"

#define	XDMA_FILE_C2H_0		L"\\c2h_0"
#define	XDMA_FILE_C2H_1		L"\\c2h_1"
#define	XDMA_FILE_C2H_2		L"\\c2h_2"
#define	XDMA_FILE_C2H_3		L"\\c2h_3"

#define XDMA_IOCTL(index) CTL_CODE(FILE_DEVICE_UNKNOWN, index + 0x800, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_XDMA_GET_VERSION			XDMA_IOCTL(0x0)
#define IOCTL_MAP_BAR					XDMA_IOCTL(0x1)


typedef struct _XDMA_BAR_INFO
{
    PVOID bar_user_virtual_address;
    ULONG bar_length;
}XDMA_BAR_INFO, * PXDMA_BAR_INFO;


#endif // !__PUBLIC_H__