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
	DirectShowTools *ds1;

private:
	bool b_grab;

private slots:
	void button_startshow_click();
	void button_grab_click();
	void paint_img(QImage src_img);
};
