#ifndef _MEDIAFOUNDATIONTOOLS_H
#define _MEDIAFOUNDATIONTOOLS_H

#include <Windows.h>
#include <sstream>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

extern "C" {
#include "jpeglib.h"
#include "jconfig.h"
#include "turbojpeg.h"
}

#include "CameraDetectTools.h"

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

	virtual int CameraInputInit(const PreviewCameraInfo& cInfo);
	virtual std::vector<CameraDeviceInfo> ListCameraDevice(); 
	virtual CameraDeviceInfo GetCameraDeviceInfo();
	virtual cv::Mat GetFrameData();
	virtual int DestoryInputParam();
	
	

	int GetDevice(std::string name);					// 打开UI界面中选择的名称的摄像头
	std::string WChar2String(LPCWSTR pwszSrc);			// 将LPCWSTR类型的字符串转换成std::string类型
	std::string ToFpsString(int framerate, int den);	// 得到帧率
	std::string GetSubType(GUID guid);					// 得到GUID对应的数据类型
	GUID GetSubType(std::string type);					// 设置摄像头输出的数据类型
	void SetDataInfo(const PreviewCameraInfo& cInfo);	// 设置摄像头工作时的参数

	void GetResolutionFromString(
		const std::string &res, 
		int &width, 
		int &height);									// 将分辨率转换成int型的长和宽
	
	cv::Mat TransformRawDataToRGB(
		IMFSample *pSample,
		uint8_t* pRGBData, 
		const int& width, 
		const int& height);								// 将原始数据类型转换成RGB24格式

	bool YUV2ToRGB24(
		uint8_t* pYUV, 
		uint8_t* pRGBData, 
		const int& width, 
		const int& height);								// 将YUV2格式的数据转换成RGB24格式

	bool I420ToBGR24(
		uint8_t* pYUV420, 
		uint8_t* pRGBData,
		const int& width, 
		const int& height);								// 将I420格式的数据转换成RGB24格式

	bool MJPGToBGR24(
		uint8_t* pMjpg, 
		const int& srcSize, 
		uint8_t* pRGBData);								// 将MJPG格式的数据转换成RGB24格式
	
private:
	IMFActivate **m_ppDevices; 
	IMFAttributes *m_pAttributes;
	IMFSourceReader* m_pReader;
	int m_deviceIndex;
	int m_width;
	int m_height;
	int m_dataTypeFlag;
	cv::Mat m_frameImg;
};

#endif