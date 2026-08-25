/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	pcie_widget.cpp
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

#include "pcie_widget.h"

#include "qcustomplot.h"

CPCIe_Widget::CPCIe_Widget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->InitData();
	this->InitUi();
	this->InitSignalSlots();
}

CPCIe_Widget::~CPCIe_Widget()
{
	m_cTimer.stop();

	if (m_pcXDMA_Test)
	{
		m_pcXDMA_Test->Exit();

		delete m_pcXDMA_Test;
		m_pcXDMA_Test = nullptr;
	}
}

void CPCIe_Widget::InitData(void)
{
	m_pcXDMA_Test = hzcc::middleware::CXDMA_Base_Factory::GetInstance()->GetPtr(hzcc::middleware::XDMA_TYPE::XILINX);
	if (m_pcXDMA_Test == nullptr)
	{
		QMessageBox::critical(this, tr("Critical dialog"), tr("Init is error."));
		exit(0);
	}

	m_nCardNum = m_pcXDMA_Test->Init();

	m_dbYAxis_LowerLimit = std::nullopt;
	m_dbYAxis_UpperLimit = std::nullopt;

	if (!m_vecCPGraph.empty())
		m_vecCPGraph.clear();

	m_vecCPGraph.push_back(ui.widgetCustomPlot->addGraph());
	m_vecCPGraph.push_back(ui.widgetCustomPlot->addGraph());

	m_vecCPGraph[0]->setName(tr("PCIe H2C"));
	m_vecCPGraph[1]->setName(tr("PCIe C2H"));

	m_vecCPGraph[0]->setPen(QPen(QColor(0, 255, 0)));
	m_vecCPGraph[1]->setPen(QPen(QColor(0, 0, 255)));

	m_cTimer.setTimerType(Qt::TimerType::VeryCoarseTimer);
	m_cTimer.setInterval(200);
}

void CPCIe_Widget::InitUi(void)
{
	QStringList strListRegSize = { tr("8 Bit"), tr("16 Bit"), tr("32 Bit") , tr("64 Bit") };

	ui.comboBox_RegSize->addItems(strListRegSize);
	ui.comboBox_RegSize->setCurrentText(tr("32 Bit"));

	////////////////////////////////数据初始化设置////////////////////////////////

	ui.widgetCustomPlot->xAxis->setLabel(tr("Time(s)"));
	ui.widgetCustomPlot->yAxis->setLabel(tr("Speed(MB/s)"));

	/* 显示子网格线 */
	ui.widgetCustomPlot->xAxis->grid()->setSubGridVisible(true);
	ui.widgetCustomPlot->yAxis->grid()->setSubGridVisible(true);

	ui.widgetCustomPlot->legend->setVisible(true);
	
	ui.widgetCustomPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

	ui.lineEdit_H2CSpeed->setText(tr("0"));
	ui.lineEdit_C2HSpeed->setText(tr("0"));

	ui.pushButton_Start->setEnabled(true);
	ui.pushButton_Stop->setEnabled(false);
}

void CPCIe_Widget::InitSignalSlots(void)
{
	connect(&m_cTimer, &QTimer::timeout, this, &CPCIe_Widget::RefreshGraph);

	connect(ui.pushButton_Start, &QPushButton::clicked, this, [this]() {
		//清楚graph数据
		for (auto& it: m_vecCPGraph)
		{
			it->data()->clear();
		}

		ui.widgetCustomPlot->xAxis->rescale();
		ui.widgetCustomPlot->yAxis->rescale();

		ui.widgetCustomPlot->replot();

		//开始h2c测试
		m_pcXDMA_Test->StartH2C_SpeedTest(0);
		m_pcXDMA_Test->StartC2H_SpeedTest(0);

		ui.lineEdit_H2CSpeed->setText(tr("0"));
		ui.lineEdit_C2HSpeed->setText(tr("0"));

		m_cTimer.start();

		ui.pushButton_Start->setEnabled(false);
		ui.pushButton_Stop->setEnabled(true);
		ui.groupBox_RegControl->setEnabled(false);
		});

	connect(ui.pushButton_Stop, &QPushButton::clicked, this, [this]() {
		m_pcXDMA_Test->StopH2C_SpeedTest();
		m_pcXDMA_Test->StopC2H_SpeedTest();

		m_cTimer.stop();

		ui.pushButton_Start->setEnabled(true);
		ui.pushButton_Stop->setEnabled(false);
		ui.groupBox_RegControl->setEnabled(true);
		});

}

void CPCIe_Widget::RefreshGraph(void)
{
	const double dbInterval = 1;

#ifndef SINGLE_MODE

	m_vecCPGraph[0]->addData(m_x, std::sin(m_x));
	m_vecCPGraph[1]->addData(m_x, std::cos(m_x));

	// 不管超了多少个，一律删掉窗口外的所有点
	double left = m_x - m_nMaxPointNum * dbInterval;

	m_vecCPGraph[0]->data()->removeBefore(left);
	m_vecCPGraph[1]->data()->removeBefore(left);

	ui.widgetCustomPlot->xAxis->setRange(left < 0 ? 0.0 : left, m_x);
	ui.widgetCustomPlot->yAxis->setRange(-2, 2);

	ui.widgetCustomPlot->replot();

	m_x += dbInterval;
#else
	auto opt_rtn_h2c = m_pcXDMA_Test->GetH2C_SpeedInfo();
	if (opt_rtn_h2c.has_value())
	{
		auto vecSpeed = opt_rtn_h2c.value();
		for (size_t i = 0; i < vecSpeed.size(); i++)
		{
			m_vecCPGraph[0]->addData(vecSpeed[i].Time / 1000.0, vecSpeed[i].Speed);

			// 不管超了多少个，一律删掉窗口外的所有点
			double left = vecSpeed[i].Speed / 1000.0 - m_nMaxPointNum * dbInterval;

			m_vecCPGraph[0]->data()->removeBefore(left);

			if (!m_dbYAxis_LowerLimit.has_value())
				m_dbYAxis_LowerLimit = vecSpeed[i].Speed;
			else
				m_dbYAxis_LowerLimit < vecSpeed[i].Speed ? 0 : m_dbYAxis_LowerLimit = vecSpeed[i].Speed;

			if (!m_dbYAxis_UpperLimit.has_value())
				m_dbYAxis_UpperLimit = vecSpeed[i].Speed;
			else
				m_dbYAxis_UpperLimit > vecSpeed[i].Speed ? 0 : m_dbYAxis_UpperLimit = vecSpeed[i].Speed;
		}
		
		ui.lineEdit_H2CSpeed->setText(QString::asprintf("%.06f", vecSpeed.back().AverageSpeed) + tr(" MB/s"));
	}

	auto opt_rtn_c2h = m_pcXDMA_Test->GetC2H_SpeedInfo();
	if (opt_rtn_c2h.has_value())
	{
		auto vecSpeed = opt_rtn_c2h.value();
		for (size_t i = 0; i < vecSpeed.size(); i++)
		{
			m_vecCPGraph[1]->addData(vecSpeed[i].Time / 1000.0, vecSpeed[i].Speed);

			// 不管超了多少个，一律删掉窗口外的所有点
			double left = vecSpeed[i].Time / 1000.0 - m_nMaxPointNum * dbInterval;

			m_vecCPGraph[1]->data()->removeBefore(left);

			ui.widgetCustomPlot->xAxis->setRange(left < 0 ? 0.0 : left, vecSpeed[i].Time / 1000.0);

			if (!m_dbYAxis_LowerLimit.has_value())
				m_dbYAxis_LowerLimit = vecSpeed[i].Speed;
			else
				m_dbYAxis_LowerLimit < vecSpeed[i].Speed ? 0 : m_dbYAxis_LowerLimit = vecSpeed[i].Speed;

			if (!m_dbYAxis_UpperLimit.has_value())
				m_dbYAxis_UpperLimit = vecSpeed[i].Speed;
			else
				m_dbYAxis_UpperLimit > vecSpeed[i].Speed ? 0 : m_dbYAxis_UpperLimit = vecSpeed[i].Speed;

			ui.widgetCustomPlot->yAxis->setRange(m_dbYAxis_LowerLimit.value_or(0.0), m_dbYAxis_UpperLimit.value_or(0.0));
		}

		ui.lineEdit_C2HSpeed->setText(QString::asprintf("%.06f", vecSpeed.back().AverageSpeed) + tr(" MB/s"));
	}

	if (opt_rtn_h2c.has_value() || opt_rtn_c2h.has_value())
		ui.widgetCustomPlot->replot();

#endif // SINGLE_MODE
}
