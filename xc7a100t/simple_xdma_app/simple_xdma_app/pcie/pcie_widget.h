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

#include "ui_pcie_widget.h"
#include "hzcc_xdma_test.h"

class QCPGraph;

class CPCIe_Widget : public QWidget
{
	Q_OBJECT

public:
    CPCIe_Widget(QWidget *parent = nullptr);
	~CPCIe_Widget();

private:
	Ui::CPCIe_WidgetClass ui;


    std::vector<QCPGraph*> m_vecCPGraph;

    QTimer m_cTimer;        //定时器

    static constexpr qint32 m_nMaxPointNum = 1200; //最大数据量
    
    hzcc::CXDMA_Test_Base* m_pcXDMA_Test = nullptr;

    std::optional<double> m_dbYAxis_LowerLimit = std::nullopt;              //y轴数据下限             
    std::optional<double> m_dbYAxis_UpperLimit = std::nullopt;              //y轴数据上限

private:
    /// <summary>
    /// 初始化ui界面
    /// </summary>
    void InitUi(void) noexcept;

    /// <summary>
    /// 初始化信号槽
    /// </summary>
    void InitSignalSlots(void) noexcept;

private:
    /// <summary>
    /// 刷新图表数据
    /// </summary>
    /// <param name=""></param>
    void RefreshGraph(void);
};
