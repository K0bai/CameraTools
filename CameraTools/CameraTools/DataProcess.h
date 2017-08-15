#ifndef _DATAPROCESS_H
#define _DATAPROCESS_H

#include "qthread.h"
#include "qimage.h"
#include <queue>
#include <Windows.h>
#include "CameraDetectTools.h"
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

	AVStream *AddVideoStream(
		AVCodecID codec_id, 
		const PreviewCameraInfo& cInfo);							// ʵ����������ļ�����
	void GetResolutionFromString(
		const std::string & res, 
		int & width, 
		int & height);												// ���ֱ��ʷֽ��int�͵ĳ��Ϳ�
	int FlushEncoder(const int& stream_index);						// ���˳�ʱ��ʣ���ͼ������������ļ�
	void WriteVideoFrame(const int &pts_num); 						// ���ļ���д���������Ƶ����
	int MP4OutputConfig(const PreviewCameraInfo& cInfo);			// �洢ΪMP4�ļ��ĳ�ʼ������
	void SaveVideo(const cv::Mat& img, const PreviewCameraInfo& cInfo);	// ����¼���������������ʵ�����������ڰ׺��ͻ�����л�
	void AddWaterMask(cv::Mat& img); 								// ���ˮӡʱ�Ĵ�����
	void GrabImage(const cv::Mat& img);									// ץ�Ĺ���������
	void PreviewImage(cv::Mat& img);								// Ԥ����������������ʵ���������ڰ׺��ͻ�����л�������ͼ������д�뻺�����
	void OptimizeImage(cv::Mat& img);								// ����Ż�������������ʵ�ֶ�ͼ�����ֵ�˲�
	void OpenOutputVideo();	 										// ������ļ��ı������Լ��������
	void DestoryVideoParam();										// ����¼��ʱ�ͷ�����������
	void FreeImgBuffer(); 											// ����ʱ�ͷ�����Mat��������
	

	void SetWaterMaskFlag(int flag);								
	void SetGrabImgFlag(int flag);
	void SetSaveVideoFlag(int flag);
	void SetImageStyle(int flag);
	void SetTransparency(int value);
	void SetInternal(int num);//
	void SetThreadStop(bool flag);//
	void SetNoiseOptimizationFlag(int flag);
	void SetWaterMaskImg(cv::Mat img);
	void SetMaskImg(cv::Mat img);
	void SetWaterMaskGifImg(std::vector<cv::Mat> img);
	void SetMaskGifImg(std::vector<cv::Mat> img);
	void SetSaveVideoPath(std::string str);
	void SetSaveGrabImgPath(std::string str);
	void SetWaterMaskRect(QRect rect);
	void SetCameraInfo(PreviewCameraInfo cInfo);
	void SetDataProcessPtr(CameraDetectTools* pCDT);
	
	
	void GetShowImageBuffer(cv::Mat& Qimg);
	void GetSaveImageBuffer(cv::Mat& img);
	std::string GetSaveVideoPath();//
	std::string GetSaveGrabImgPath();
	PreviewCameraInfo GetCameraInfo();
	AVFormatContext *GetOutputFormatCtx();
	AVFrame *GetOutputFrame();
	QRect GetWaterMaskRect();
	int GetImageStyle();
	int GetInternal();//
	int GetGrabImgFlag();
	int GetSaveVideoFlag();//
	bool GetThreadStop();//

protected:
	void run();								// ��ȡ������洢���ݵĹ����߳�
signals:
	void SendUpdataImgMsg();				// ���͸��½���ͼ����ź�
	void SendPaintImgMsg();					// ��OpenGL�з��ͻ���ͼ����ź�
	void SendImageGrabMsg();				// ����ץ��ͼ���ź�
	void SendAbort(int ret);				// ����������ֹ�ź�

private slots:
	void SaveGrabImage();					// ����ץͼ�źţ����д�ͼ

private:
	CameraDetectTools *m_pCDT;
	AVCodecContext *m_outputCodecCtx;
	AVFormatContext *m_outputFormatCtx;
	AVFrame *m_outputFrame;
	AVStream *m_outputVideoStream;
	AVOutputFormat *m_outputFormat;
	uint8_t *m_pictureBuff;
	uint8_t *m_videoOutBuff;
	int m_videoOutBuffSize;
	int m_waterMaskFlag;
	int m_transparency;
	int m_showIndex;
	int m_internal;
	int m_grabImgFlag;
	int m_saveVideoFlag;
	int m_noiseOptimizationFlag;
	int m_ptsNum;
	int m_imageStyle;
	bool b_threadStop;
	QRect m_waterMaskRect;
	cv::Mat m_waterMaskImg;
	cv::Mat m_maskImg;
	cv::Mat m_grabImgMat;
	cv::Mat m_saveImgMat;
	std::vector<cv::Mat> m_waterMaskGifImg;
	std::vector<cv::Mat> m_maskGifImg;
	std::queue<cv::Mat> m_showImageBuf;
	std::queue<cv::Mat> m_saveImageBuf;
	std::string m_saveVideoPath;
	std::string m_saveGrabImgPath;
	PreviewCameraInfo m_cameraInfo;
	CRITICAL_SECTION m_previewLock;
	CRITICAL_SECTION m_saveBufferLock;
};

#endif