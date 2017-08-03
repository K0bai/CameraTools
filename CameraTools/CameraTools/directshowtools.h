#ifndef DIRECTSHOWTOOLS_H
#define DIRECTSHOWTOOLS_H

#include <Windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <atlconv.h>
#include <set>
#include <map>
#include <iterator>
#include <dshow.h>
#include <OleAuto.h>
#include <objbase.h>

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

struct classCompare
{
	bool operator()(const std::string& lhs, const std::string& rhs)
	{
		return std::atoi(lhs.c_str()) > std::atoi(rhs.c_str());
	}
};

struct CameraDeviceInfo {
    std::string friend_name;
    std::string moniker_name;
    std::set<std::string> data_type;
    std::set<std::string, classCompare> data_resolution;
    std::set<std::string, classCompare> data_bit;
    std::set<std::string, classCompare> data_fps;
    std::vector< std::vector<std::string> > data;
};

typedef struct tagVIDEOINFOHEADER2 {
    RECT             rcSource;
    RECT             rcTarget;
    DWORD            dwBitRate;
    DWORD            dwBitErrorRate;
    REFERENCE_TIME   AvgTimePerFrame;
    DWORD            dwInterlaceFlags;
    DWORD            dwCopyProtectFlags;
    DWORD            dwPictAspectRatioX;
    DWORD            dwPictAspectRatioY;
    union {
        DWORD dwControlFlags;
        DWORD dwReserved1;
    };
    DWORD            dwReserved2;
    BITMAPINFOHEADER bmiHeader;
} VIDEOINFOHEADER2;


class DirectShowTools : public QThread
{
	Q_OBJECT

public:
    DirectShowTools();
    // 将图像数据编码为MP4的相关函数，暂时需要多参数
	int MP4OutputConfig(const std::string& output_name, int& picture_size, uint8_t* picture_buf);
	AVStream *add_video_stream(AVFormatContext *oc, enum AVCodecID codec_id);
	void open_video(AVFormatContext *oc, AVStream *st);
	void write_video_frame(AVFormatContext *oc_m, AVStream *video_st_m, int &pts_num,
						   AVFrame* &m_OutputFrame, int &framecnt);

    // 从摄像头获取图像数据的函数
    int CameraInputInit(const std::string& camera_name, int &video_stream);

    // 在退出时将剩余的图像数据输出到文件
    int flush_encoder(unsigned int stream_index);

    int GetMajorType(GUID guid, char* buffer);
    int GuidToString(const GUID &guid, char* buffer);
    int GetSubType(GUID guid, char* buffer);
    int GetFormatType(GUID guid, char* buffer);

    AVCodecContext *GetInputCodecCtx() { return m_InputCodecCtx; }
    AVCodecContext *GetOutputCodecCtx() { return m_OutputCodecCtx; }
    AVFormatContext *GetInputFormatCtx() { return m_InputFormatCtx; }
    AVFormatContext *GetOutputFormatCtx() { return m_OutputFormatCtx; }
    AVFrame *GetInputFrame() { return m_InputFrame; }
    AVFrame *GetInputFrameRGB() { return m_InputFrameRGB; }
    AVFrame *GetOutputFrame() { return m_OutputFrame; }
    AVStream *GetOutputVideoStream() { return m_OutputVideoStream; }

    CameraDeviceInfo GetCameraDeviceInfo(IMoniker* pMoniker);
    std::vector<CameraDeviceInfo> ListCameraDevice();
	void ConfigCamera();

    void Destory();

protected:
	void run();
signals:
	void send_image_data(QImage src_img);
private:
    // 从摄像头获取数据的变量
    AVCodecContext *m_InputCodecCtx;
    AVFormatContext *m_InputFormatCtx;
    AVFrame *m_InputFrame;
    AVFrame *m_InputFrameRGB;

    // h264编码图像数据的变量
    AVCodecContext *m_OutputCodecCtx;
    AVFormatContext *m_OutputFormatCtx;
    AVFrame *m_OutputFrame;
    AVStream *m_OutputVideoStream;
	AVOutputFormat *m_OutputFormat;


};

#endif // DIRECTSHOWTOOLS_H
