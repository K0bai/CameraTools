#ifndef _DIRECTSHOWTOOLS_H
#define _DIRECTSHOWTOOLS_H

#include "qthread.h"
#include "qimage.h"
#include "cameraconfig.h"
#include <queue>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

// 相机初始化
#define DST_CAMERA_INIT_ERROR -1
#define DST_CAMERA_INIT_OK 0

// 抓拍图像相关的状态参数
#define DST_GRAB_READY 0
#define DST_GRAB_PROCESSING 1
#define DST_GRAB_SAVING 2

// 添加水印相关状态参数
#define DST_WATERMASK_NONE 0
#define DST_WATERMASK_RGB 1
#define DST_WATERMASK_GIF 2

// 录制视频相关状态参数
#define DST_SAVEVIDEO_NONE 0
#define DST_SAVEVIDEO_INIT 1
#define DST_SAVEVIDEO_SAVING 2
#define DST_SAVEVIDEO_END 3

// 录像初始化状态
#define DST_SAVEVIDEO_INIT_ERROR -1
#define DST_SAVEVIDEO_INIT_OK 0

// 图像风格
#define DST_IMAGESTYLE_NORMAL 0
#define DST_IMAGESTYLE_GRAY 1
#define DST_IMAGESTYLE_OIL 2

struct PreviewCameraInfo {
	std::string name;
	std::string type;
	std::string resolution;
	std::string bit;
	std::string fps;
};

class DirectShowTools : public QThread
{
	Q_OBJECT

public:
    DirectShowTools();
	void AddWaterMask(cv::Mat& img);					 // 添加水印时的处理函数
	int MP4OutputConfig(const std::string& output_name); // 存储为MP4文件的初始化配置

	/* 
	 * 引用几个函数修改自muxing.c 文档
	 */
	AVStream *AddVideoStream(enum AVCodecID codec_id);   
	void OpenOutputVideo();								 
	void WriteVideoFrame(int &pts_num);
	int FlushEncoder(unsigned int stream_index);		// 在退出时将剩余的图像数据输出到文件
	void DestoryInputParam();							
	void DestoryVideoParam();
	int CameraInputInit(PreviewCameraInfo cInfo, int &video_stream); // 获取摄像头图像数据的初始化函数
	int GetImageData(int& video_stream, AVPacket& packet, SwsContext* &img_convert_ctx);
	void SaveVideo(cv::Mat& img);
	void GrabImage(cv::Mat& img);
	void PreviewImage(cv::Mat& img);

	void SetWaterMaskFlag(int flag);
	void SetTransparency(int value);
	void SetWaterMaskImg(cv::Mat img);
	void SetMaskImg(cv::Mat img);
	void SetWaterMaskGifImg(std::vector<cv::Mat> img);
	void SetMaskGifImg(std::vector<cv::Mat> img);
	void SetInternal(int num);//
	int GetInternal();//
	void SetGrabImgFlag(int flag);
	int GetGrabImgFlag();
	void SetSaveVideoFlag(int flag);
	int GetSaveVideoFlag();//
	void SetCameraInfo(PreviewCameraInfo cInfo);
	PreviewCameraInfo GetCameraInfo();
	void SetSaveVideoPath(std::string str);
	std::string GetSaveVideoPath();//
	void SetThreadStop(bool flag);//
	bool GetThreadStop();//
	void GetImageBuffer(QImage& Qimg);
	void SetImageStyle(int flag);

protected:
	void run();								// 获取数据与存储数据的工作线程
signals:
	void SendUpdataImgMsg();		 
	void SendImageGrabMsg();				// 发送抓拍图像信号给指定槽函数
	void SendUnexpectedAbort();				// 发送意外终止信号

private slots:
	void SaveGrabImage();					// 接受抓图信号，进行存图

private:
    AVCodecContext *m_InputCodecCtx;
    AVFormatContext *m_InputFormatCtx;
    AVFrame *m_InputFrame;
    AVFrame *m_InputFrameRGB;
    AVCodecContext *m_OutputCodecCtx;
    AVFormatContext *m_OutputFormatCtx;
    AVFrame *m_OutputFrame;
    AVStream *m_OutputVideoStream;
	AVOutputFormat *m_OutputFormat;
	uint8_t* m_PictureBuff;
	uint8_t* m_VideoOutBuff;
	int m_VideoOutBuffSize;
	int m_WaterMaskFlag;
	int m_Transparency;
	int m_ShowIndex;
	int m_Internal;
	int m_GrabImgFlag;
	int m_SaveVideoFlag;
	int m_PtsNum;
	cv::Mat m_WaterMaskImg;
	cv::Mat m_MaskImg;
	cv::Mat m_GrabImgMat;
	std::vector<cv::Mat> m_WaterMaskGifImg;
	std::vector<cv::Mat> m_MaskGifImg;
	PreviewCameraInfo m_CameraInfo;
	std::string m_SaveVideoPath;
	bool b_ThreadStop;	
	int m_ImageStyle;
	std::queue<QImage> m_ImageBuf;
	CRITICAL_SECTION m_lock;
};

#endif // DIRECTSHOWTOOLS_H
