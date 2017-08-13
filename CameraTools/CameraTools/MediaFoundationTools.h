#ifndef _MEDIAFOUNDATIONTOOLS_H
#define _MEDIAFOUNDATIONTOOLS_H

#include <Windows.h>
#include <sstream>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include "CameraDetectTools.h"

extern "C" {
#include "jpeglib.h"
#include "jconfig.h"
#include "turbojpeg.h"
}

// 用于判断从摄像头获取的数据类型，然后进行数据转换
#define MFT_DATATYPE_RGB24 0
#define MFT_DATATYPE_YUV2 1
#define MFT_DATATYPE_I420 2
#define MFT_DATATYPE_MJPG 3

class MediaFoundationTools : public CameraDetectTools
{
public:
	MediaFoundationTools();
	~MediaFoundationTools();

	std::vector<CameraDeviceInfo> ListCameraDevice(); 
	CameraDeviceInfo GetCameraDeviceInfo();  
	cv::Mat GetFrameData();
	int CameraInputInit(const PreviewCameraInfo& cInfo);
	int DestoryInputParam();

	int SetDevice(std::string name);

	std::string WChar2String(LPCWSTR pwszSrc);
	std::string ToFpsString(int framerate, int den);
	std::string GetSubType(GUID guid);
	GUID SetSubType(std::string type);
	void SetDataInfo(const PreviewCameraInfo& cInfo);
	void GetResolutionFromString(const std::string &res, int &width, int &height);
	
	cv::Mat TransformRawDataToRGB(IMFSample *pSample, uint8_t* pRGBData, const int& width, const int& height);
	bool YUV2_To_RGB24(uint8_t* pYUV, uint8_t* pRGBData, const int& width, const int& height);
	bool I420_To_BGR24(uint8_t* pYUV420, uint8_t* pRGBData, const int& width, const int& height);
	bool MJPG_To_BGR24(uint8_t* pMjpg, const int& srcSize, uint8_t* pRGBData);
	
private:
	IMFActivate **ppDevices; 
	IMFAttributes *pAttributes;
	IMFSourceReader* reader;
	int m_DeviceIndex;
	int m_Width;
	int m_Height;
	int m_DataTypeFlag;
	cv::Mat m_FrameImg;
};

#endif