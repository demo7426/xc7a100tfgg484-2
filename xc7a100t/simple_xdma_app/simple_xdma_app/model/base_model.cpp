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

#include "base_model.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        void CIBase_Model::Validate(std::vector<VALIDATOR_ERROR>& errors)
        {
        }

        void CIBase_Model::SaveToXml(QDomElement& dom, QDomNode& node) const
        {
        }

        void CIBase_Model::LoadFormXml(QDomElement& dom)
        {
        }

        void CIBase_Model::SaveToExcel() const
        {
        }

        void CIBase_Model::LoadFormExcel()
        {
        }

    }
}
