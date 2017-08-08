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

// 相机初始化结果参数
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

// 录像初始化结果参数
#define DST_SAVEVIDEO_INIT_ERROR -1
#define DST_SAVEVIDEO_INIT_OK 0

// 图像风格状态参数
#define DST_IMAGESTYLE_NORMAL 0
#define DST_IMAGESTYLE_GRAY 1
#define DST_IMAGESTYLE_OIL 2

// 与摄像头相关的一套配置参数
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
	~DirectShowTools();
	AVStream *AddVideoStream(enum AVCodecID codec_id);   // 实现配置输出文件参数
	
	int MP4OutputConfig(const std::string& output_name); // 存储为MP4文件的初始化配置
	int FlushEncoder(unsigned int stream_index);		 // 在退出时将剩余的图像数据输出到文件
	int CameraInputInit(PreviewCameraInfo cInfo, int &video_stream); // 获取摄像头图像数据的初始化函数
	int GetImageData(int& video_stream, AVPacket& packet, SwsContext* &img_convert_ctx); // 获取摄像头图像数据
	void AddWaterMask(cv::Mat& img);					 // 添加水印时的处理函数
	void OpenOutputVideo();								 // 打开输出文件的编码器以及编码参数
	void WriteVideoFrame(int &pts_num);					 // 向文件中写入编码后的视频数据
	void SaveVideo(cv::Mat& img);						 // 本地录像的主函数，里面实现了正常，黑白和油画风格切换
	void GrabImage(cv::Mat& img);						 // 抓拍功能主函数
	void PreviewImage(cv::Mat& img);					 // 预览主函数，在里面实现正常，黑白和油画风格切换，并将图像数据写入缓冲队列
	void DestoryInputParam();							 // 结束预览时释放相关输入参数
	void DestoryVideoParam();							 // 结束录像时释放相关输入参数
	void FreeImgBuffer();								 // 结束时释放所有Mat数据类型

	void SetWaterMaskFlag(int flag);
	void SetTransparency(int value);
	void SetWaterMaskImg(cv::Mat img);
	void SetMaskImg(cv::Mat img);
	void SetWaterMaskGifImg(std::vector<cv::Mat> img);
	void SetMaskGifImg(std::vector<cv::Mat> img);
	void SetGrabImgFlag(int flag);
	void SetSaveVideoFlag(int flag);
	void SetCameraInfo(PreviewCameraInfo cInfo);
	void SetSaveVideoPath(std::string str);
	void GetImageBuffer(QImage& Qimg);
	void SetImageStyle(int flag);
	void SetThreadStop(bool flag);//
	void SetInternal(int num);//
	int GetInternal();//
	int GetGrabImgFlag();
	int GetSaveVideoFlag();//
	bool GetThreadStop();//
	PreviewCameraInfo GetCameraInfo();
	std::string GetSaveVideoPath();//

protected:
	void run();								// 获取数据与存储数据的工作线程
signals:
	void SendUpdataImgMsg();				// 发送更新界面图像的信号
	void SendImageGrabMsg();				// 发送抓拍图像信号
	void SendAbort(int ret);				// 发送意外终止信号

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
	int m_ImageStyle;
	bool b_ThreadStop;
	cv::Mat m_WaterMaskImg;
	cv::Mat m_MaskImg;
	cv::Mat m_GrabImgMat;
	std::vector<cv::Mat> m_WaterMaskGifImg;
	std::vector<cv::Mat> m_MaskGifImg;
	std::queue<QImage> m_ImageBuf;
	std::string m_SaveVideoPath;
	PreviewCameraInfo m_CameraInfo;
	CRITICAL_SECTION m_PreviewLock;
};

#endif // DIRECTSHOWTOOLS_H
