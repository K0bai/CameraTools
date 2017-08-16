#ifndef _CAMERATOOLS_H
#define _CAMERATOOLS_H

#include <QtWidgets/QMainWindow>
#include "ui_CameraTools.h"
#include <QMouseEvent>

#include "DataProcess.h"
#include "CameraDetectTools.h"
#include "DirectShowTools.h"
#include "MediaFoundationTools.h"
#include "FreeImage.h"

// ���¿ؼ���ز���
#define CT_STARTPREVIEW 0
#define CT_STOPPREVIEW 1

// ˮӡ��ز���
#define CT_WATERMASK_ERROR -1
#define CT_WATERMASK_OK 0
#define CT_WATERMASK_MAX_HEIGHT 200
#define CT_WATERMASK_MIN_WIDTH 200

// ����ͷ��⹤����ز���
#define  CT_DETECTTOOLS_DS 0
#define  CT_DETECTTOOLS_MF 1
#define  CT_DETECTTOOLS_VFW 2

// �Ƿ��ƶ�ˮӡ
#define CT_MOVEWATERMASK_NO 0
#define CT_MOVEWATERMASK_YES 1

class CameraTools : public QMainWindow
{
	Q_OBJECT

public:
	CameraTools(QWidget *parent = Q_NULLPTR);
	PreviewCameraInfo GetCameraParam();			 // �õ��ӽ������õ�����ͷ����
	void DetectCamera();						 // ���ڼ��ϵͳ�п���ʹ�õ�����ͷ
	void WriteRecord(QString msg);				 // �����־��¼
	void UpdateControl(int flag);				 // ���½���ؼ�

	void ShowCameraInfo(
		const std::vector<CameraDeviceInfo>& camera_info,
		const int& index);						 // ����չʾ����������ͷ�ľ�������

	void StartPreview(DataProcess* pDP);		 // ��ʼԤ��������������⹤����ָ�봫��

	void GetResolutionFromString(
		const std::string &res,
		int &width,
		int &height);							 // ���ֱ���ת����int�͵ĳ��Ϳ�

	void BitToMat(
		FIBITMAP* fiBmp, 
		const FREE_IMAGE_FORMAT &fif,
		cv::Mat& gifImg, 
		cv::Mat& maskImg);						 // �������ڽ�FIBITMAP*��ʽ������ת��Ϊcv::Mat������

	int GifToMat(
		std::vector<cv::Mat>& gifImgs, 
		std::vector<cv::Mat>& maskImgs, 
		std::string filename);					 // ���ڶ�ȡGIF��ʽͼ�񲢽���������ת��Ϊcv::Mat

	int IsWaterMaskSizeOk(
		const int& height, 
		const int& width);						 // �����ж����ˮӡ��С�Ƿ�Ϸ�

protected:
	bool nativeEvent(const QByteArray &eventType,
					 void *message, 
					 long *result);				 // ����Ӳ���Ȱβ��¼�����
	void mouseMoveEvent(QMouseEvent *event);	 // ����ƶ��¼�����
	void mousePressEvent(QMouseEvent *event);	 // ������¼�����
private:
	Ui::CameraToolsClass ui;
	
	CameraDetectTools *m_pCDT;
	DataProcess *m_dp;

	QPoint m_startPoint;
	QPoint m_pointOffset;

	bool b_cameraInitState;
	int m_detectTools;
	int m_captureState;
	int m_isMoveWaterMask;
	std::vector<CameraDeviceInfo> m_cameraList;

private slots:
	void button_startshow_click();		// ���տ�ʼԤ����ťclick��Ϣ
	void button_grab_click();			// ����ץ�İ�ťclick��Ϣ
	void button_addmask_click();		// �������ˮӡ��ťclick��Ϣ
	void button_startcapture_click();	// ���տ�ʼ¼��ťclick��Ϣ
	void paint_img();					// ���ջ���ͼ�����Ϣ
	void combobox_camera_change();		// ����������б������仯�¼�
	void combobox_imagestyle_change();	// ����ͼ�����б������仯�¼�
	void combobox_detecttools_change(); // ��������ͷ��⹤���б������仯�¼�
	void slider_value_change();			// ����ˮӡ͸���Ȼ���ֵ�仯�¼�
	void get_abort(int msg);			// ������ֹԤ������Ϣ
	void checkbox_filter();				// ��������Ż�����Ϣ
};

#endif