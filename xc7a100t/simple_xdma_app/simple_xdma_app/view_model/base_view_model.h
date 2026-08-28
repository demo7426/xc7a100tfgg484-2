/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	base_view_model.h
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

#pragma once

#include <memory>

#include <QObject>
#include <QDomDocument>

#include "validataor.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        class CIBase_Model;

        class CIBase_View_Model : public QObject
        {
            Q_OBJECT

        public:
            explicit CIBase_View_Model(QObject* parent = nullptr);
            virtual ~CIBase_View_Model() = 0;

            /// <summary>
            /// 初始化model
            /// </summary>
            /// <param name="model"></param>
            virtual void InitModel(std::shared_ptr<CIBase_Model> model);

        };
    }

}


