/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	pcie_config_space_model.cpp
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

#include <cstring>

#include "pcie_config_space_model.h"

namespace hzcc
{
    namespace simple_xdma_app
    {
        CIPCIe_Config_Space_Model::CIPCIe_Config_Space_Model() 
        {
            
        }

        void CIPCIe_Config_Space_Model::Validate(std::vector<VALIDATOR_ERROR>& errors)
        {
        }

        void CIPCIe_Config_Space_Model::SaveToXml(QDomElement& dom, QDomNode& node) const
        {
        }

        void CIPCIe_Config_Space_Model::LoadFormXml(QDomElement& dom)
        {
        }

        void CIPCIe_Config_Space_Model::SaveToExcel() const
        {
        }

        void CIPCIe_Config_Space_Model::LoadFormExcel()
        {
        }

        void CIPCIe_Config_Space_Model::RefreshData(FILED_TYPE type)
        {
            if (type != FILED_TYPE::PCIE_CONFIG_SPACE && type != FILED_TYPE::ALL)
                return;


            //TODO:从硬件读取所有寄存器
            this->set_VendorID(0x10ee);
            this->set_DeviceID(0x7022);

            emit allDataChanged();
        }

    }
}
