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

#include <memory>

#include "base_model.h"
#include "singleton.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        class CIProject_Model : public CIBase_Model
        {
        public:
            CIProject_Model();
            virtual ~CIProject_Model() {};

            /// <summary>
            /// 保存数据到xml
            /// </summary>
            /// <param name="file_path"></param>
            void SaveToXml(std::string file_path) const;

            /// <summary>
            /// 从xml加载数据
            /// </summary>
            /// <param name="dom"></param>
            void LoadFormXml(std::string file_path);

            /// <summary>
            /// 保存数据到xml
            /// </summary>
            /// <param name="file_path"></param>
            void SaveToExcel(std::string file_path) const;

            /// <summary>
            /// 从xml加载数据
            /// </summary>
            /// <param name="file_path"></param>
            void LoadFormExcel(std::string file_path);

            /// <summary>
            /// 数据校验
            /// </summary>
            /// <param name=""></param>
            void Validate(std::vector<VALIDATOR_ERROR>& errors) final;

        private:

            /// <summary>
            /// 刷新数据
            /// </summary>
            /// <param name="type"></param>
            void RefreshData(FILED_TYPE type) final;

        private:
            void InitModel();

            void AddSubModel(FILED_TYPE type, std::shared_ptr<CIBase_Model> base_model);

            void RemoveSubModel(FILED_TYPE type);

            std::map<FILED_TYPE, std::shared_ptr<CIBase_Model>> m_mapBase;
        };

        class CIProject_Model_Factory : public CSingleton<CIProject_Model_Factory>
        {
            friend class CSingleton<CIProject_Model_Factory>;
        public:
            ~CIProject_Model_Factory() {}

            std::shared_ptr<CIBase_Model> Create(FILED_TYPE type);
        };
    }
}
