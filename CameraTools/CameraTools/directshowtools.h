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

	virtual int CameraInputInit(const PreviewCameraInfo& cInfo);
	virtual std::vector<CameraDeviceInfo> ListCameraDevice();	
	virtual CameraDeviceInfo GetCameraDeviceInfo();
	virtual cv::Mat GetFrameData(int& flag);
	virtual int DestoryInputParam();

	std::string GetSubType(GUID guid);			// 得到摄像头设备的GUID对应的类型
	

private:
	IMoniker *m_pMoniker;
	AVCodecContext *m_inputCodecCtx;
	AVFormatContext *m_inputFormatCtx;
	AVFrame *m_inputFrame;
	AVFrame *m_inputFrameRGB;
	AVPacket m_packet;
	int m_videoStream;
};

#endif