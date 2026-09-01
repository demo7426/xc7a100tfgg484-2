/*************************************************
Copyright (C), 2009-2012    , Level Chip Co., Ltd.
文件名:	public.h
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

#include <ntddk.h>
#include <wdf.h>
#include <wdm.h>
#include <initguid.h>

#define BAR_MAX_NUM    6       //Bar的最大数量


// 74c7e4a9-6d5d-4a70-bc0d-20691dff9e9d
DEFINE_GUID(GUID_DEVINTERFACE_XDMA,
    0x74c7e4a9, 0x6d5d, 0x4a70, 0xbc, 0x0d, 0x20, 0x69, 0x1d, 0xff, 0x9e, 0x9d);



#endif // !__PUBLIC_H__