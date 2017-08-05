#ifndef _CAMERATOOLS_H
#define _CAMERATOOLS_H

#include <QtWidgets/QMainWindow>
#include "ui_CameraTools.h"
#include "directshowtools.h"
#include "cameraconfig.h"
#include "FreeImage.h"

class CameraTools : public QMainWindow
{
	Q_OBJECT

public:
	CameraTools(QWidget *parent = Q_NULLPTR);
	void DetectCamera();
	void ShowCameraInfo(std::vector<CameraDeviceInfo>& camera_info, int index);
	int GifToMat(std::vector<cv::Mat>& gifImgs, const char* filename);
	cv::Mat BitToMat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif);

signals:
	void ThreadStop();

protected:
	bool nativeEvent(const QByteArray &eventType, void *message, long *result);

private:
	Ui::CameraToolsClass ui;
	DirectShowTools *ds1;

private:
	bool b_camera_state;
	bool b_grab;
	std::vector<CameraDeviceInfo> camera_device_list;

private slots:
	void button_startshow_click();
	void button_grab_click();
	void paint_img(QImage src_img);
	void combobox_camera_change();
	void button_addmask_click();
	void slider_value_change();
};

#endif