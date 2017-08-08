#ifndef _CAMERATOOLS_H
#define _CAMERATOOLS_H

#include <QtWidgets/QMainWindow>
#include "ui_CameraTools.h"
#include "directshowtools.h"
#include "cameraconfig.h"
#include "FreeImage.h"

// ���¿ؼ���ز���
#define CT_STARTPREVIEW 0
#define CT_STOPPREVIEW 1

// ˮӡ��ز���
#define CT_WATERMASK_ERROR -1
#define CT_WATERMASK_OK 0
#define CT_WATERMASK_MAX_HEIGHT 200
#define CT_WATERMASK_MIN_WIDTH 200

class CameraTools : public QMainWindow
{
	Q_OBJECT

public:
	CameraTools(QWidget *parent = Q_NULLPTR);
	void DetectCamera();					// ���ڼ��ϵͳ�п���ʹ�õ�����ͷ
	void WriteRecord(QString msg);			// �����־��¼
	void UpdateControl(int flag);			// ���½���ؼ�
	void ShowCameraInfo(std::vector<CameraDeviceInfo>& camera_info, int index); // ����չʾ����������ͷ�ľ�������
	PreviewCameraInfo GetCameraParam();		// �õ��ӽ������õ�����ͷ����

	/*
	 * ���������������ڴ���GIFͼ����Ϊˮӡ�������
	 * BitToMat�������ڽ�FIBITMAP*��ʽ������ת��Ϊcv::Mat�����ݡ�
	 * GifToMat���ڶ�ȡGIF��ʽͼ�񲢽���������ת��Ϊcv::Mat
	 */
	void BitToMat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif,
					 cv::Mat& gifImg, cv::Mat& maskImg);
	int GifToMat(std::vector<cv::Mat>& gifImgs, std::vector<cv::Mat>& maskImgs, 
				const char* filename);
	int IsWaterMaskSizeOk(int height, int width);	// �����ж����ˮӡ��С�Ƿ�Ϸ�

protected:
	bool nativeEvent(const QByteArray &eventType,
					 void *message, long *result);	// ����Ӳ���Ȱβ��¼�����

private:
	Ui::CameraToolsClass ui;
	DirectShowTools *m_Dst;
	bool b_CameraInitState;
	int m_CaptureState;
	std::vector<CameraDeviceInfo> m_CameraList;

private slots:
	void button_startshow_click();		// ���տ�ʼԤ����ťclick��Ϣ
	void button_grab_click();			// ����ץ�İ�ťclick��Ϣ
	void button_addmask_click();		// �������ˮӡ��ťclick��Ϣ
	void button_startcapture_click();	// ���տ�ʼ¼��ťclick��Ϣ
	void paint_img();					// ���ջ���ͼ�����Ϣ
	void combobox_camera_change();		// ����������б������仯�¼�
	void combobox_imagestyle_change();	// ����ͼ�����б������仯�¼�
	void slider_value_change();			// ����ˮӡ͸���Ȼ���ֵ�仯�¼�
	void get_abort(int);				// ������ֹԤ������Ϣ
};

#endif