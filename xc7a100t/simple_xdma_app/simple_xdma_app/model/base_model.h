/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	base_model.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.24
描  述: mvvm model层基类
备  注:	
修改记录:

  1.  日期: 2026.08.24
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#pragma once

#include <QDomDocument>

#include "validataor.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        class CIXMLSerializeable
        {
        public:
            CIXMLSerializeable() = default;
            virtual ~CIXMLSerializeable() {}

            /// <summary>
            /// 保存数据到xml
            /// </summary>
            /// <param name="dom"></param>
            virtual void SaveToXml(QDomElement& dom, QDomNode& node) const = 0;

            /// <summary>
            /// 从xml加载数据
            /// </summary>
            /// <param name="dom"></param>
            virtual void LoadFormXml(QDomElement& dom) = 0;

        };
        
        class CIExcelSerializeable
        {
        public:
            CIExcelSerializeable() = default;
            virtual ~CIExcelSerializeable(){}

            /// <summary>
            /// 保存数据到excel
            /// </summary>
            /// <param name="dom"></param>
            virtual void SaveToExcel() const = 0;

            /// <summary>
            /// 从excel加载数据
            /// </summary>
            /// <param name="dom"></param>
            virtual void LoadFormExcel() = 0;

        };

        class CIBase_Model: public CIXMLSerializeable, public CIExcelSerializeable, public CIValidataor
        {
        public:
            CIBase_Model() = default;
            virtual ~CIBase_Model() = default;

            /// <summary>
            /// 数据校验
            /// </summary>
            /// <param name=""></param>
            virtual void Validate(std::vector<VALIDATOR_ERROR>& errors) override;

            /// <summary>
            /// 保存数据到xml
            /// </summary>
            /// <param name="dom"></param>
            virtual void SaveToXml(QDomElement& dom, QDomNode& node) const override;

            /// <summary>
            /// 从xml加载数据
            /// </summary>
            /// <param name="dom"></param>
            virtual void LoadFormXml(QDomElement& dom) override;

            /// <summary>
            /// 保存数据到excel
            /// </summary>
            /// <param name="dom"></param>
            virtual void SaveToExcel() const override;

            /// <summary>
            /// 从excel加载数据
            /// </summary>
            /// <param name="dom"></param>
            virtual void LoadFormExcel() override;

            /// <summary>
            /// 刷新数据
            /// </summary>
            /// <param name="type"></param>
            virtual void RefreshData(FILED_TYPE type) = 0;
        };
    }

}


