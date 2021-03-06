#include "ExampleWidget.h"
#include <QPainter>
#include <QTimer>

ExampleWidget::ExampleWidget(QWidget* parent)
	: QWidget(parent)
	, spec(0, 4000, 500)                        //创建0-40000HZ的频谱，采样点为100,其中负值为左声道，正值为右声道
{
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, static_cast<void (QWidget::*)()>(&QWidget::repaint));
	timer->setInterval(17);                             //设置刷新间隔为17ms，约为60帧
	timer->start();
}

ExampleWidget::~ExampleWidget()
{
}

void ExampleWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.translate(0, height());
	painter.scale(1, -1);                //翻转坐标系

	float offset = width() / (float)spec.getBarCount();
	QPen pen(QColor(0, 100, 200), 5);
	painter.setPen(pen);
	auto specData = spec.getBars();
	for (int i = 0; i < specData.size(); i++) {
		painter.drawLine(QPointF(i * offset, 0), QPointF(i * offset, specData[i] * height()));
	}

	////显示频谱节奏均值
	//pen.setBrush(QColor(107,105,214));
	//painter.setPen(pen);
	//painter.drawLine(QPointF(0,height()*0.98),QPointF(width()*SpecEngine::getEngine()->getRmsLevel(),height()*0.98));

	////显示频谱节奏峰值
	//pen.setBrush(QColor(146,189,108));
	//painter.setPen(pen);
	//painter.drawLine(QPointF(0,height()*0.95),QPointF(width()*SpecEngine::getEngine()->getPeakLevel(),height()*0.95));

	////显示频谱节奏脉冲
	//pen.setBrush(QColor(252,1,26));
	//painter.setPen(pen);
	//painter.drawLine(QPointF(0,height()*0.92),QPointF(width()*SpecEngine::getEngine()->getPulse(),height()*0.92));
}