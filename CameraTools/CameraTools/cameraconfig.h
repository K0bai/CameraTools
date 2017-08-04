#ifndef _CAMERACONFIG_H
#define _CAMERACONFIG_H

#include <Windows.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iterator>
#include <algorithm>

#include <atlconv.h>
#include <dshow.h>
#include <OleAuto.h>
#include <objbase.h>

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

class CameraConfig
{
public:
	CameraConfig();
	~CameraConfig();

public:
	// 从摄像头获取图像数据的函数
	
	CameraDeviceInfo GetCameraDeviceInfo(IMoniker* pMoniker);
	void ConfigCamera();

	int GetMajorType(GUID guid, char* buffer);
	int GuidToString(const GUID &guid, char* buffer);
	int GetSubType(GUID guid, char* buffer);
	int GetFormatType(GUID guid, char* buffer);

public:
	std::vector<CameraDeviceInfo> ListCameraDevice();
};

#endif