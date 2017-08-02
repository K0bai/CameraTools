#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CameraTools.h"
#include "directshowtools.h"

class CameraTools : public QMainWindow
{
	Q_OBJECT

public:
	CameraTools(QWidget *parent = Q_NULLPTR);
	void ShowCameraInfo(std::vector<CameraDeviceInfo>& camera_info, int index);

private:
	Ui::CameraToolsClass ui;

private slots:
	void button_startshow_click();
};
