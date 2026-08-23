/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	pcie_model.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.24
描  述: pcie model
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
#include "error_model.h"

class CIPCIe_Model : public CIBase_Model, public CIError_Model
{
public:
    CIPCIe_Model() = default;
    virtual ~CIPCIe_Model() {};

    /// <summary>
    /// 保存数据到xml
    /// </summary>
    /// <param name="dom"></param>
    void SaveToXml(QDomElement& dom) override;

    /// <summary>
    /// 从xml加载数据
    /// </summary>
    /// <param name="dom"></param>
    void LoadForXml(QDomElement& dom) override;

    /// <summary>
    /// 数据校验
    /// </summary>
    /// <param name=""></param>
    void Validate(std::vector<ERROR_INFO>& errors) override;

private:

};
