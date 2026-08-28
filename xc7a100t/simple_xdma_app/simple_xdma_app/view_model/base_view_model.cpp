/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	base_view_model.cpp
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.29
描  述: mvvm view model层基类
备  注:
修改记录:

  1.  日期: 2026.08.29
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#include "base_view_model.h"
#include "base_model.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        CIBase_View_Model::CIBase_View_Model(QObject* parent) : QObject(parent)
        {
        }

        CIBase_View_Model::~CIBase_View_Model(){}

        void CIBase_View_Model::InitModel(std::shared_ptr<CIBase_Model> model)
        {
        }

    }
}
