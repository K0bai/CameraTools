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

// 相机初始化结果参数
#define DP_CAMERA_INIT_ERROR -1
#define DP_CAMERA_INIT_OK 0

// 抓拍图像相关的状态参数
#define DP_GRAB_READY 0
#define DP_GRAB_PROCESSING 1
#define DP_GRAB_SAVING 2

// 添加水印相关状态参数
#define DP_WATERMASK_NONE 0
#define DP_WATERMASK_RGB 1
#define DP_WATERMASK_GIF 2

// 录制视频相关状态参数
#define DP_SAVEVIDEO_NONE 0
#define DP_SAVEVIDEO_INIT 1
#define DP_SAVEVIDEO_SAVING 2
#define DP_SAVEVIDEO_END 3

// 录像初始化结果参数
#define DP_SAVEVIDEO_INIT_ERROR -1
#define DP_SAVEVIDEO_INIT_OK 0

// 图像风格状态参数
#define DP_IMAGESTYLE_NORMAL 0
#define DP_IMAGESTYLE_GRAY 1
#define DP_IMAGESTYLE_OIL 2

// 图像噪点优化参数
#define DP_NOISEOPTIMIZATION_NO 0
#define DP_NOISEOPTIMIZATION_YES 1


class DataProcess : public QThread
{
	Q_OBJECT

public:
	DataProcess();
	~DataProcess();

	AVStream *AddVideoStream(AVCodecID codec_id, const PreviewCameraInfo& cInfo);   // 实现配置输出文件参数
	void GetResolutionFromString(const std::string & res, int & width, int & height);
	int MP4OutputConfig(const PreviewCameraInfo& cInfo);							// 存储为MP4文件的初始化配置
	int FlushEncoder(const int& stream_index);									// 在退出时将剩余的图像数据输出到文件
	void AddWaterMask(cv::Mat& img);												// 添加水印时的处理函数
	void OpenOutputVideo();															// 打开输出文件的编码器以及编码参数
	void WriteVideoFrame(const int &pts_num);										// 向文件中写入编码后的视频数据
	void SaveVideo(cv::Mat& img, const PreviewCameraInfo& cInfo);					// 本地录像的主函数，里面实现了正常，黑白和油画风格切换
//	void SaveVideoThread();															// 录像的线程函数，用于从缓冲队列中读取数据并存储
	void GrabImage(cv::Mat& img);													// 抓拍功能主函数
	void PreviewImage(cv::Mat& img);												// 预览主函数，在里面实现正常，黑白和油画风格切换，并将图像数据写入缓冲队列
	void OptimizeImage(cv::Mat& img);
	void DestoryVideoParam();														// 结束录像时释放相关输入参数
	void FreeImgBuffer();															// 结束时释放所有Mat数据类型
	

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
	void run();								// 获取数据与存储数据的工作线程
signals:
	void SendUpdataImgMsg();				// 发送更新界面图像的信号
	void SendImageGrabMsg();				// 发送抓拍图像信号
	void SendAbort(int ret);				// 发送意外终止信号

private slots:
	void SaveGrabImage();					// 接受抓图信号，进行存图

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