#ifndef _CAMERATOOLS_H
#define _CAMERATOOLS_H

#include <QtWidgets/QMainWindow>
#include "ui_CameraTools.h"
#include <exception>
#include "DataProcess.h"
#include "CameraDetectTools.h"
#include "DirectShowTools.h"
#include "VFWTools.h"
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

// �����Ƿ���л�ͼ
#define  CT_ISPAINTING_NO 0
#define  CT_ISPAINTING_YES 1


class CameraTools : public QMainWindow
{
	Q_OBJECT

public:
	CameraTools(QWidget *parent = Q_NULLPTR);
	void DetectCamera();	// ���ڼ��ϵͳ�п���ʹ�õ�����ͷ
	void WriteRecord(QString msg);			// �����־��¼
	void UpdateControl(int flag);			// ���½���ؼ�
	void ShowCameraInfo(const std::vector<CameraDeviceInfo>& camera_info,
						const int& index);  // ����չʾ����������ͷ�ľ�������
	void StartPreview(DataProcess* pDP);
	PreviewCameraInfo GetCameraParam();		// �õ��ӽ������õ�����ͷ����

	/*
	 * ���������������ڴ���GIFͼ����Ϊˮӡ�������
	 * BitToMat�������ڽ�FIBITMAP*��ʽ������ת��Ϊcv::Mat�����ݡ�
	 * GifToMat���ڶ�ȡGIF��ʽͼ�񲢽���������ת��Ϊcv::Mat
	 */
	void BitToMat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif,
					 cv::Mat& gifImg, cv::Mat& maskImg);
	int GifToMat(std::vector<cv::Mat>& gifImgs, std::vector<cv::Mat>& maskImgs, 
				 std::string filename);
	int IsWaterMaskSizeOk(const int& height, const int& width);	// �����ж����ˮӡ��С�Ƿ�Ϸ�

protected:
	bool nativeEvent(const QByteArray &eventType,
					 void *message, long *result);	// ����Ӳ���Ȱβ��¼�����

private:
	Ui::CameraToolsClass ui;
	DataProcess *m_Dst;
	CameraDetectTools *pCDT_DS;
	CameraDetectTools *pCDT_MF;
	CameraDetectTools *pCDT_VFW;
	CameraDetectToolsInterface* pCDTI_DS;
	CameraDetectToolsInterface* pCDTI_MF;
	CameraDetectToolsInterface* pCDTI_VFW;

	int m_DetectTools;
	bool b_CameraInitState;
	int m_CaptureState;
	int m_IsPainting;
	std::vector<CameraDeviceInfo> m_CameraList;

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