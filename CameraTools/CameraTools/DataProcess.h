#ifndef _DATAPROCESS_H
#define _DATAPROCESS_H

#include "qthread.h"
#include "qimage.h"
#include "cameraconfig.h"
#include <queue>
#include "CameraDetectTools.h"
#include <Windows.h>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

// �����ʼ���������
#define DP_CAMERA_INIT_ERROR -1
#define DP_CAMERA_INIT_OK 0

// ץ��ͼ����ص�״̬����
#define DP_GRAB_READY 0
#define DP_GRAB_PROCESSING 1
#define DP_GRAB_SAVING 2

// ���ˮӡ���״̬����
#define DP_WATERMASK_NONE 0
#define DP_WATERMASK_RGB 1
#define DP_WATERMASK_GIF 2

// ¼����Ƶ���״̬����
#define DP_SAVEVIDEO_NONE 0
#define DP_SAVEVIDEO_INIT 1
#define DP_SAVEVIDEO_SAVING 2
#define DP_SAVEVIDEO_END 3

// ¼���ʼ���������
#define DP_SAVEVIDEO_INIT_ERROR -1
#define DP_SAVEVIDEO_INIT_OK 0

// ͼ����״̬����
#define DP_IMAGESTYLE_NORMAL 0
#define DP_IMAGESTYLE_GRAY 1
#define DP_IMAGESTYLE_OIL 2

// ͼ������Ż�����
#define DP_NOISEOPTIMIZATION_NO 0
#define DP_NOISEOPTIMIZATION_YES 1


class DataProcess : public QThread
{
	Q_OBJECT

public:
	DataProcess();
	~DataProcess();

	AVStream *AddVideoStream(AVCodecID codec_id, const PreviewCameraInfo& cInfo);   // ʵ����������ļ�����
	void GetResolutionFromString(const std::string & res, int & width, int & height);
	int MP4OutputConfig(const PreviewCameraInfo& cInfo);							// �洢ΪMP4�ļ��ĳ�ʼ������
	int FlushEncoder(const int& stream_index);									// ���˳�ʱ��ʣ���ͼ������������ļ�
	void AddWaterMask(cv::Mat& img);												// ���ˮӡʱ�Ĵ�����
	void OpenOutputVideo();															// ������ļ��ı������Լ��������
	void WriteVideoFrame(const int &pts_num);										// ���ļ���д���������Ƶ����
	void SaveVideo(cv::Mat& img, const PreviewCameraInfo& cInfo);					// ����¼���������������ʵ�����������ڰ׺��ͻ�����л�
//	void SaveVideoThread();															// ¼����̺߳��������ڴӻ�������ж�ȡ���ݲ��洢
	void GrabImage(cv::Mat& img);													// ץ�Ĺ���������
	void PreviewImage(cv::Mat& img);												// Ԥ����������������ʵ���������ڰ׺��ͻ�����л�������ͼ������д�뻺�����
	void OptimizeImage(cv::Mat& img);
	void DestoryVideoParam();														// ����¼��ʱ�ͷ�����������
	void FreeImgBuffer();															// ����ʱ�ͷ�����Mat��������
	

	void SetWaterMaskFlag(int flag);
	void SetTransparency(int value);
	void SetWaterMaskImg(cv::Mat img);
	void SetMaskImg(cv::Mat img);
	void SetWaterMaskGifImg(std::vector<cv::Mat> img);
	void SetMaskGifImg(std::vector<cv::Mat> img);
	void SetGrabImgFlag(int flag);
	void SetSaveVideoFlag(int flag);
	void SetNoiseOptimizationFlag(int flag);
	void SetCameraInfo(PreviewCameraInfo cInfo);
	void SetSaveVideoPath(std::string str);
	void GetShowImageBuffer(cv::Mat& Qimg);
	void GetSaveImageBuffer(cv::Mat& img);
	void SetImageStyle(int flag);
	void SetCameraToolsInterface(CameraDetectToolsInterface* pCDTI);
	int GetImageStyle();
	void SetThreadStop(bool flag);//
	void SetInternal(int num);//
	int GetInternal();//
	int GetGrabImgFlag();
	int GetSaveVideoFlag();//
	bool GetThreadStop();//
	PreviewCameraInfo GetCameraInfo();
	std::string GetSaveVideoPath();//

	AVFormatContext *GetOutputFormatCtx();
	AVFrame *GetOutputFrame();

protected:
	void run();								// ��ȡ������洢���ݵĹ����߳�
signals:
	void SendUpdataImgMsg();				// ���͸��½���ͼ����ź�
	void SendImageGrabMsg();				// ����ץ��ͼ���ź�
	void SendAbort(int ret);				// ����������ֹ�ź�

private slots:
	void SaveGrabImage();					// ����ץͼ�źţ����д�ͼ

private:
	CameraDetectToolsInterface *m_pCDTI;
	AVCodecContext *m_OutputCodecCtx;
	AVFormatContext *m_OutputFormatCtx;
	AVFrame *m_OutputFrame;
	AVStream *m_OutputVideoStream;
	AVOutputFormat *m_OutputFormat;
	uint8_t *m_PictureBuff;
	uint8_t *m_VideoOutBuff;
	int m_VideoOutBuffSize;
	int m_WaterMaskFlag;
	int m_Transparency;
	int m_ShowIndex;
	int m_Internal;
	int m_GrabImgFlag;
	int m_SaveVideoFlag;
	int m_NoiseOptimizationFlag;
	int m_PtsNum;
	int m_ImageStyle;
	bool b_ThreadStop;
	cv::Mat m_WaterMaskImg;
	cv::Mat m_MaskImg;
	cv::Mat m_GrabImgMat;
	cv::Mat m_SaveImgMat;
	std::vector<cv::Mat> m_WaterMaskGifImg;
	std::vector<cv::Mat> m_MaskGifImg;
	std::queue<cv::Mat> m_ShowImageBuf;
	std::queue<cv::Mat> m_SaveImageBuf;
	std::string m_SaveVideoPath;
	PreviewCameraInfo m_CameraInfo;
	CRITICAL_SECTION m_PreviewLock;
	CRITICAL_SECTION m_SaveBufferLock;
};

#endif