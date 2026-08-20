#pragma once

#include <QWidget>
#include <vector>

#include "ui_pcie_widget.h"

class QCustomPlot;
class QCPGraph;

class CPCIe_Widget : public QWidget
{
	Q_OBJECT

public:
    CPCIe_Widget(QWidget *parent = nullptr);
	~CPCIe_Widget();

private:
	Ui::CPCIe_WidgetClass ui;

    QHBoxLayout* m_pcHBoxLayout = nullptr;

	QCustomPlot* m_pcCustomPlot = nullptr;
    std::vector<QCPGraph*> m_vecCPGraph;

    QTimer m_cTimer;        //定时器

    double m_x = 0;

    static constexpr qint32 m_nMaxPointNum = 2000; //最大数据量
    
private:
    /// <summary>
    /// 初始化ui界面
    /// </summary>
    void InitUi(void) noexcept;

    /// <summary>
    /// 初始化信号槽
    /// </summary>
    void InitSignalSlots(void) noexcept;
};
