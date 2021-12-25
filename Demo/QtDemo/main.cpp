#include "ExampleWidget.h"
#include <QApplication>
#include "SpectrumEngine.h"

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	auto it = SpectrumEngine::getInstance()->enumDevices();
	SpectrumEngine::getInstance()->start(it.front());
	ExampleWidget w;
	w.show();
	a.exec();
	SpectrumEngine::getInstance()->stop();
	return 0;
}