/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	pcie_config_space_model.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.25
描  述: pcie_config_space mdoel
备  注:
修改记录:

  1.  日期: 2026.08.25
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#pragma once

#include <QWidget>

class CPCIe_Config_Space_Model : public QWidget
{
	Q_OBJECT

public:
    CPCIe_Config_Space_Model(QWidget *parent = nullptr);
	~CPCIe_Config_Space_Model();


private:
};
