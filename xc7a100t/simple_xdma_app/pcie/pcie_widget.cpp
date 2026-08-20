#include "pcie_widget.h"

#include "qcustomplot.h"

CPCIe_Widget::CPCIe_Widget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->InitUi();
	this->InitSignalSlots();
}

CPCIe_Widget::~CPCIe_Widget()
{
	if (m_pcHBoxLayout)
	{
		delete m_pcHBoxLayout;
		m_pcHBoxLayout = nullptr;
	}
}

void CPCIe_Widget::InitUi(void) noexcept
{
	////////////////////////////////ui布局////////////////////////////////
	m_pcCustomPlot = new QCustomPlot(this);

	m_pcHBoxLayout = new QHBoxLayout();

	m_pcHBoxLayout->addWidget(m_pcCustomPlot);

	this->setLayout(m_pcHBoxLayout);

	////////////////////////////////数据初始化设置////////////////////////////////

	m_x = 0;

	if(!m_vecCPGraph.empty())
		m_vecCPGraph.clear();

	m_vecCPGraph.push_back(m_pcCustomPlot->addGraph());
	m_vecCPGraph.push_back(m_pcCustomPlot->addGraph());

	m_vecCPGraph[0]->setName(tr("PCIe H2C"));
	m_vecCPGraph[1]->setName(tr("PCIe C2H"));

	m_vecCPGraph[0]->setPen(QPen(QColor(255, 0, 0)));
	m_vecCPGraph[1]->setPen(QPen(QColor(0, 255, 0)));

	m_pcCustomPlot->xAxis->setLabel(tr("Time(s)"));
	m_pcCustomPlot->yAxis->setLabel(tr("Speed(MB/s)"));

	/* 显示子网格线 */
	m_pcCustomPlot->xAxis->grid()->setSubGridVisible(true);
	m_pcCustomPlot->yAxis->grid()->setSubGridVisible(true);

	m_pcCustomPlot->legend->setVisible(true);
	
	m_pcCustomPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

	m_cTimer.setInterval(1);
}

void CPCIe_Widget::InitSignalSlots(void) noexcept
{
	connect(&m_cTimer, &QTimer::timeout, this, [=](){
		const double dbInterval = 0.01;

		m_vecCPGraph[0]->addData(m_x, std::sin(m_x));
		m_vecCPGraph[1]->addData(m_x, std::cos(m_x));

		// 不管超了多少个，一律删掉窗口外的所有点
		double left = m_x - m_nMaxPointNum * dbInterval;

		m_vecCPGraph[0]->data()->removeBefore(left);
		m_vecCPGraph[1]->data()->removeBefore(left);

		m_pcCustomPlot->xAxis->setRange(left < 0 ? 0.0 : left, m_x);
		m_pcCustomPlot->yAxis->setRange(-2, 2);

		m_pcCustomPlot->replot();

		m_x += dbInterval;

		});

	m_cTimer.start();
}
