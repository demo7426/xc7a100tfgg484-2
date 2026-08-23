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

#pragma once

#include <QDomDocument>

class CIBase_Model
{
public:
	CIBase_Model() = default;
	virtual ~CIBase_Model() = 0;

    /// <summary>
    /// 保存数据到xml
    /// </summary>
    /// <param name="dom"></param>
    virtual void SaveToXml(QDomElement& dom) = 0;

    /// <summary>
    /// 从xml加载数据
    /// </summary>
    /// <param name="dom"></param>
    virtual void LoadForXml(QDomElement& dom) = 0;
    

private:

};
