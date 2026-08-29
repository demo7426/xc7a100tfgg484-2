/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	project_model.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.24
描  述: 整个项目的model
备  注:	
修改记录:

  1.  日期: 2026.08.24
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#pragma once

#include "base_view_model.h"
#include "singleton.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        class CIProject_View_Model : public CIBase_View_Model
        {
        public:
            CIProject_View_Model();
            virtual ~CIProject_View_Model() {};


            /// <summary>
            /// 获取子 View Model，供 View 初始化使用
            /// </summary>
            inline std::shared_ptr<CIBase_View_Model> GetSubViewModel(FILED_TYPE type) const
            {
                auto it = m_mapBase.find(type);
                return (it != m_mapBase.end()) ? it->second : nullptr;
            }

        private:
            void InitViewModel();

            void AddSubViewModel(FILED_TYPE type, std::shared_ptr<CIBase_View_Model> base_model);

            void RemoveSubViewModel(FILED_TYPE type);

            std::map<FILED_TYPE, std::shared_ptr<CIBase_View_Model>> m_mapBase;
        };

        class CIProject_View_Model_Factory : public CSingleton<CIProject_View_Model_Factory>
        {
            friend class CSingleton<CIProject_View_Model_Factory>;

        public:
            ~CIProject_View_Model_Factory() {}

            std::shared_ptr<CIBase_View_Model> Create(FILED_TYPE type);
        };
    }
}
