/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	project_model.cpp
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

#include "project_view_model.h"
#include "pcie_config_space_view_model.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        hzcc::simple_xdma_app::CIProject_View_Model::CIProject_View_Model()
        {
        }

        void CIProject_View_Model::InitViewModel()
        {
            this->AddSubViewModel(FILED_TYPE::PCIE_BAR, CIProject_View_Model_Factory::GetInstance()->Create(FILED_TYPE::PCIE_BAR));
            this->AddSubViewModel(FILED_TYPE::PCIE_CONFIG_SPACE, CIProject_View_Model_Factory::GetInstance()->Create(FILED_TYPE::PCIE_CONFIG_SPACE));
            this->AddSubViewModel(FILED_TYPE::PCIE_XMDA, CIProject_View_Model_Factory::GetInstance()->Create(FILED_TYPE::PCIE_XMDA));
        }

        void CIProject_View_Model::AddSubViewModel(FILED_TYPE type, std::shared_ptr<CIBase_View_Model> base_model)
        {
            m_mapBase[type] = base_model;
        }

        void CIProject_View_Model::RemoveSubViewModel(FILED_TYPE type)
        {
            m_mapBase.erase(type);
        }

        std::shared_ptr<CIBase_View_Model> CIProject_View_Model_Factory::Create(FILED_TYPE type)
        {
            switch (type)
            {
            case hzcc::simple_xdma_app::FILED_TYPE::NONE:
                break;
            case hzcc::simple_xdma_app::FILED_TYPE::PCIE_BAR:
                return nullptr;     //TODO:暂未支持
            case hzcc::simple_xdma_app::FILED_TYPE::PCIE_CONFIG_SPACE:
                return std::make_shared<CPCIe_Config_Space_View_Model>();
            case hzcc::simple_xdma_app::FILED_TYPE::PCIE_XMDA:
                return nullptr;     //TODO:暂未支持
            default:
                break;
            }

            return nullptr;
        }
    }
}
