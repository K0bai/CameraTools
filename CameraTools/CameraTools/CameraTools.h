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
	void DetectCamera();														// ���ڼ��ϵͳ�п���ʹ�õ�����ͷ
	void ShowCameraInfo(std::vector<CameraDeviceInfo>& camera_info, int index); // ����չʾ����������ͷ�ľ�������
	PreviewCameraInfo GetCameraParam();

	/*
	 * ���������������ڴ���GIFͼ����Ϊˮӡ�������
	 * BitToMat�������ڽ�FIBITMAP*��ʽ������ת��Ϊcv::Mat�����ݲ����ء�
	 * GifToMat���ڶ�ȡGIF��ʽͼ�񲢽���������ת��Ϊcv::Mat
	 */
	cv::Mat BitToMat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif);
	int GifToMat(std::vector<cv::Mat>& gifImgs, const char* filename);			
			

signals:
	void ThreadStop();

protected:
	bool nativeEvent(const QByteArray &eventType, void *message, long *result); // ����Ӳ���Ȱβ��¼�����

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