#ifndef _DIRECTSHOWTOOLS_H
#define _DIRECTSHOWTOOLS_H

#include "CameraDetectTools.h"
#include <atlconv.h>
#include <dshow.h>
#include <dvdmedia.h>
#include <wmsdkidl.h>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class DirectShowTools : public CameraDetectTools
{
public:
	DirectShowTools();
	~DirectShowTools();

	std::vector<CameraDeviceInfo> ListCameraDevice();	 
	CameraDeviceInfo GetCameraDeviceInfo();			 
	std::string GetSubType(GUID guid); 
	cv::Mat GetFrameData();
	int CameraInputInit(const PreviewCameraInfo& cInfo);
	int DestoryInputParam();

private:
	IMoniker *pMoniker;
	AVCodecContext *m_InputCodecCtx;
	AVFormatContext *m_InputFormatCtx;
	AVFrame *m_InputFrame;
	AVFrame *m_InputFrameRGB;
	AVPacket packet;
	int video_stream;
};

#endif