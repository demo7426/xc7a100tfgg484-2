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

#include "project_model.h"
#include "pcie_config_space_model.h"
#include "pcie_xdma_model.h"
#include "pcie_bar_model.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        CIProject_Model::CIProject_Model()
        {
            this->InitModel();
        }

        void CIProject_Model::SaveToXml(std::string file_path) const
        {
        }

        void CIProject_Model::LoadFormXml(std::string file_path)
        {
        }

        void CIProject_Model::SaveToExcel(std::string file_path) const
        {
        }

        void CIProject_Model::LoadFormExcel(std::string file_path)
        {
        }

        void CIProject_Model::InitModel()
        {
            this->AddSubModel(FILED_TYPE::PCIE_BAR, CIProject_Model_Factory::GetInstance()->Create(FILED_TYPE::PCIE_BAR));
            this->AddSubModel(FILED_TYPE::PCIE_CONFIG_SPACE, CIProject_Model_Factory::GetInstance()->Create(FILED_TYPE::PCIE_CONFIG_SPACE));
            this->AddSubModel(FILED_TYPE::PCIE_XMDA, CIProject_Model_Factory::GetInstance()->Create(FILED_TYPE::PCIE_XMDA));
        }

        void CIProject_Model::AddSubModel(FILED_TYPE type, std::shared_ptr<CIBase_Model> base_model)
        {
            m_mapBase[type] = base_model;
        }

        void CIProject_Model::RemoveSubModel(FILED_TYPE type)
        {
            m_mapBase.erase(type);
        }

        void CIProject_Model::Validate(std::vector<VALIDATOR_ERROR>& errors)
        {
            for (auto& it: m_mapBase)
            {
                it.second->Validate(errors);
            }
        }

        void CIProject_Model::RefreshData(FILED_TYPE type)
        {
            for (auto& it : m_mapBase)
            {
                it.second->RefreshData(type);
            }
        }

        std::shared_ptr<CIBase_Model> CIProject_Model_Factory::Create(FILED_TYPE type)
        {
            switch (type)
            {
            case hzcc::simple_xdma_app::FILED_TYPE::NONE:
                break;
            case hzcc::simple_xdma_app::FILED_TYPE::PCIE_BAR:
                return std::make_shared<CIPCIe_Bar_Model>();
            case hzcc::simple_xdma_app::FILED_TYPE::PCIE_CONFIG_SPACE:
                return std::make_shared<CIPCIe_Config_Space_Model>();
            case hzcc::simple_xdma_app::FILED_TYPE::PCIE_XMDA:
                return std::make_shared<CIPCIe_XDMA_Model>();
            default:
                break;
            }

            return nullptr;
        }
    }
}
