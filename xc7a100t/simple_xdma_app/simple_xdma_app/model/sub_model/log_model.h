/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	log_model.h
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

#include "base_model.h"


namespace hzcc
{
    namespace simple_xdma_app
    {

        class CILog_Model : public CIBase_Model
        {
        public:
            CILog_Model() = default;
            virtual ~CILog_Model() {};

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
            /// 保存数据到xml
            /// </summary>
            /// <param name="dom"></param>
            virtual void SaveToExcel() const override;

            /// <summary>
            /// 从xml加载数据
            /// </summary>
            /// <param name="dom"></param>
            virtual void LoadFormExcel() override;

            /// <summary>
            /// 刷新数据
            /// </summary>
            /// <param name="type"></param>
            virtual void RefreshData(FILED_TYPE type) override;
        };
    }
}
