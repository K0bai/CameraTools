#ifndef _DIRECTSHOWTOOLS_H
#define _DIRECTSHOWTOOLS_H

#include "qthread.h"
#include "qimage.h"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>


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

class DirectShowTools : public QThread
{
	Q_OBJECT

public:
	
	// 0表示不添加水印，1表示添加单张图片水印，2表示添加gif水印
	int m_MaskFlag;
	int m_Transparency;

	// 水印普通图像
	cv::Mat m_WaterMaskImg;
	cv::Mat m_MaskImg;
	
	// 水印gif图像
	std::vector<cv::Mat> m_WaterMaskGifImg;
	int m_ShowIndex;
	int m_Internal;

	// 抓拍图像
	cv::Mat m_GrabImgMat;
	int m_GrabImgFlag;

	int m_SaveVideoFlag;
	int m_PtsNum;

	std::string m_CameraName;
	std::string m_SaveVideoPath;

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

	int CameraInputInit(const std::string& camera_name, int &video_stream); // 获取摄像头图像数据的初始化函数
    
	int GetImageData(int& video_stream, AVPacket& packet, SwsContext* &img_convert_ctx);
	void SaveVideo(cv::Mat& img);
	void GrabImage(cv::Mat& img);
	void PreviewImage(cv::Mat& img);
protected:
	void run();								// 获取数据与存储数据的工作线程
signals:
	void SendImageData(QImage src_img);		// 将图像数据发送到主UI进行显示
	void SendImageGrabMsg();				// 发送抓拍图像信号给指定槽函数

private slots:
	void ThreadStopFunc();					// 停止获取图像数据线程
	void SaveGrabImage();					// 接受抓图信号，进行存图

private:
	/*
	 * 从摄像头获取图像数据的相关参数
	 */
    AVCodecContext *m_InputCodecCtx;
    AVFormatContext *m_InputFormatCtx;
    AVFrame *m_InputFrame;
    AVFrame *m_InputFrameRGB;

	

	/*
	 * MP4编码相关的参数
	 */
    AVCodecContext *m_OutputCodecCtx;
    AVFormatContext *m_OutputFormatCtx;
    AVFrame *m_OutputFrame;
    AVStream *m_OutputVideoStream;
	AVOutputFormat *m_OutputFormat;
	uint8_t* m_PictureBuff;
	uint8_t* m_VideoOutBuff;
	int m_VideoOutBuffSize;

	
	bool m_ThreadStop;						// 停止获取图像数据线程的参数
};

#endif // DIRECTSHOWTOOLS_H
