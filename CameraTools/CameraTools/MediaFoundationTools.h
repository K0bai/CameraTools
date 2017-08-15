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

// �����жϴ�����ͷ��ȡ���������ͣ�Ȼ���������ת��
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
	
	

	int GetDevice(std::string name);					// ��UI������ѡ������Ƶ�����ͷ
	std::string WChar2String(LPCWSTR pwszSrc);			// ��LPCWSTR���͵��ַ���ת����std::string����
	std::string ToFpsString(int framerate, int den);	// �õ�֡��
	std::string GetSubType(GUID guid);					// �õ�GUID��Ӧ����������
	GUID GetSubType(std::string type);					// ��������ͷ�������������
	void SetDataInfo(const PreviewCameraInfo& cInfo);	// ��������ͷ����ʱ�Ĳ���

	void GetResolutionFromString(
		const std::string &res, 
		int &width, 
		int &height);									// ���ֱ���ת����int�͵ĳ��Ϳ�
	
	cv::Mat TransformRawDataToRGB(
		IMFSample *pSample,
		uint8_t* pRGBData, 
		const int& width, 
		const int& height);								// ��ԭʼ��������ת����RGB24��ʽ

	bool YUV2ToRGB24(
		uint8_t* pYUV, 
		uint8_t* pRGBData, 
		const int& width, 
		const int& height);								// ��YUV2��ʽ������ת����RGB24��ʽ

	bool I420ToBGR24(
		uint8_t* pYUV420, 
		uint8_t* pRGBData,
		const int& width, 
		const int& height);								// ��I420��ʽ������ת����RGB24��ʽ

	bool MJPGToBGR24(
		uint8_t* pMjpg, 
		const int& srcSize, 
		uint8_t* pRGBData);								// ��MJPG��ʽ������ת����RGB24��ʽ
	
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