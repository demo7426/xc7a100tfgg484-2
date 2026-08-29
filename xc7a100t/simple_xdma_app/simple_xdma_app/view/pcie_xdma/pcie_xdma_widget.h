/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	pcie_widget.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.21
描  述: pcie相关控制窗口
备  注:	
修改记录:

  1.  日期: 2026.08.21
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#pragma once

#include <QWidget>
#include <vector>
#include <optional>

#include "ui_pcie_xdma_widget.h"
#include "hzcc_middleware_xdma_factory.h"
#include "base_view.h"

class QCPGraph;

namespace hzcc
{
    namespace simple_xdma_app
    {
        class CIBase_View_Model;

    }
}


class CPCIe_XDMA_Widget : public QWidget, public hzcc::simple_xdma_app::CIBase_View
{
	Q_OBJECT

public:
    CPCIe_XDMA_Widget(QWidget *parent = nullptr);
	~CPCIe_XDMA_Widget();

    /// <summary>
    /// 初始化model
    /// </summary>
    /// <param name="model"></param>
    virtual void InitViewModel(std::shared_ptr<hzcc::simple_xdma_app::CIBase_View_Model> model) override;

    /// <summary>
    /// 获取卡数量
    /// </summary>
    /// <param name=""></param>
    /// <returns>卡数量</returns>
    inline int GetCardNum(void)
    {
        return m_nCardNum;
    }

private:
	Ui::CPCIe_XDMA_WidgetClass ui;


    std::vector<QCPGraph*> m_vecCPGraph;

    QTimer m_cTimer;        //定时器

    static constexpr qint32 m_nMaxPointNum = 1200; //最大数据量
    
    hzcc::middleware::CXDMA_Base* m_pcXDMA_Test = nullptr;

    std::optional<double> m_dbYAxis_LowerLimit = std::nullopt;              //y轴数据下限             
    std::optional<double> m_dbYAxis_UpperLimit = std::nullopt;              //y轴数据上限

    int m_nCardNum = 0;         //卡数量

    //std::weak_ptr<hzcc::simple_xdma_app::CIBase_View_Model> m_view_model;

private:
    /// <summary>
    /// 初始化数据
    /// </summary>
    /// <param name=""></param>
    void InitData(void);

    /// <summary>
    /// 初始化ui界面
    /// </summary>
    void InitUi(void);

    /// <summary>
    /// 初始化信号槽
    /// </summary>
    void InitSignalSlots(void);

private:
    /// <summary>
    /// 刷新图表数据
    /// </summary>
    /// <param name=""></param>
    void RefreshGraph(void);
};
