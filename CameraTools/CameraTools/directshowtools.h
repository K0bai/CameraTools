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

public:
    DirectShowTools();
	// 水印相关
	void AddWaterMask(cv::Mat& img);



    // 将图像数据编码为MP4的相关函数，暂时需要多参数
	int MP4OutputConfig(const std::string& output_name);
	AVStream *add_video_stream(enum AVCodecID codec_id);
	void open_video();
	void write_video_frame(int &pts_num);

    

    // 在退出时将剩余的图像数据输出到文件
    int flush_encoder(unsigned int stream_index);

	int CameraInputInit(const std::string& camera_name, int &video_stream);

    AVCodecContext *GetInputCodecCtx() { return m_InputCodecCtx; }
    AVCodecContext *GetOutputCodecCtx() { return m_OutputCodecCtx; }
    AVFormatContext *GetInputFormatCtx() { return m_InputFormatCtx; }
    AVFormatContext *GetOutputFormatCtx() { return m_OutputFormatCtx; }
    AVFrame *GetInputFrame() { return m_InputFrame; }
    AVFrame *GetInputFrameRGB() { return m_InputFrameRGB; }
    AVFrame *GetOutputFrame() { return m_OutputFrame; }
    AVStream *GetOutputVideoStream() { return m_OutputVideoStream; }

    void Destory();

protected:
	void run();
signals:
	void send_image_data(QImage src_img);

private slots:
	void ThreadStopFunc();

private:
    // 从摄像头获取数据的变量
    AVCodecContext *m_InputCodecCtx;
    AVFormatContext *m_InputFormatCtx;
    AVFrame *m_InputFrame;
    AVFrame *m_InputFrameRGB;

	

    // mp4编码图像数据的变量
    AVCodecContext *m_OutputCodecCtx;
    AVFormatContext *m_OutputFormatCtx;
    AVFrame *m_OutputFrame;
    AVStream *m_OutputVideoStream;
	AVOutputFormat *m_OutputFormat;
	uint8_t* m_PictureBuff;
	uint8_t* m_VideoOutBuff;
	int m_VideoOutBuffSize;

	// 停止线程控制变量
	bool m_ThreadStop;
};

#endif // DIRECTSHOWTOOLS_H
