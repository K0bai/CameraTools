#include "CameraTools.h"
#include <QtWidgets/QApplication>

#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "Quartz.lib")

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	CameraTools w;
	w.show();
	return a.exec();
}
