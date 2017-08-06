#ifndef _CAMERATOOLS_H
#define _CAMERATOOLS_H

#include <QtWidgets/QMainWindow>
#include "ui_CameraTools.h"
#include "directshowtools.h"
#include "cameraconfig.h"
#include "FreeImage.h"

struct PreviewCameraInfo{
	std::string name;							
	std::string type;					
	std::string resolution; 
	std::string bit;		
	std::string fps;		
};

class CameraTools : public QMainWindow
{
	Q_OBJECT

public:
	CameraTools(QWidget *parent = Q_NULLPTR);
	void DetectCamera();														// 用于检测系统中可以使用的摄像头
	void ShowCameraInfo(std::vector<CameraDeviceInfo>& camera_info, int index); // 用于展示检测出的摄像头的具体数据
	PreviewCameraInfo GetCameraParam();

	/*
	 * 下面两个函数用于处理GIF图像作为水印的情况。
	 * BitToMat函数用于将FIBITMAP*格式的数据转换为cv::Mat的数据并返回。
	 * GifToMat用于读取GIF格式图像并将数据内容转换为cv::Mat
	 */
	cv::Mat BitToMat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif);
	int GifToMat(std::vector<cv::Mat>& gifImgs, const char* filename);			
			

signals:
	void ThreadStop();

protected:
	bool nativeEvent(const QByteArray &eventType, void *message, long *result); // 用于硬件热拔插事件处理

private:
	Ui::CameraToolsClass ui;
	DirectShowTools *m_Dst;
	bool b_CameraState;
	std::vector<CameraDeviceInfo> m_CameraList;

private slots:
	void button_startshow_click();
	void button_grab_click();
	void button_addmask_click();
	void button_startcapture_click();
	void paint_img(QImage src_img);
	void combobox_camera_change();
	void slider_value_change();
};

#endif