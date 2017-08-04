#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CameraTools.h"
#include "directshowtools.h"
#include "cameraconfig.h"

class CameraTools : public QMainWindow
{
	Q_OBJECT

public:
	CameraTools(QWidget *parent = Q_NULLPTR);
	void ShowCameraInfo(std::vector<CameraDeviceInfo>& camera_info, int index);

signals:
	void ThreadStop();

private:
	Ui::CameraToolsClass ui;
	DirectShowTools *ds1;

private:
	bool b_grab;
	std::vector<CameraDeviceInfo> camera_device_list;

private slots:
	void button_startshow_click();
	void button_grab_click();
	void paint_img(QImage src_img);
	void combobox_camera_change();
	void button_addmask_click();
};
