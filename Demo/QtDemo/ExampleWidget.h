#ifndef EXAMPLEWIDGET_H
#define EXAMPLEWIDGET_H

#include <QWidget>
#include "SpectrumProvider.h"

class ExampleWidget : public QWidget
{
	Q_OBJECT
public:
	ExampleWidget(QWidget* parent = nullptr);
	~ExampleWidget();
private:
	//需要使用频谱，只需创建一个Spectrum对象
	SpectrumProvider spec;
protected:
	virtual void paintEvent(QPaintEvent* event) override;
};
#endif // EXAMPLEWIDGET_H
