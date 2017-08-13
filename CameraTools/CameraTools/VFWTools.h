#ifndef _VFWTOOLS_H
#define _VFWTOOLS_H

#include "CameraDetectTools.h"
#include <Windows.h>
#include "vfw.h"
#include <tchar.h>


class VFWTools : public CameraDetectTools
{
public:
	VFWTools();
	~VFWTools();

	std::vector<CameraDeviceInfo> ListCameraDevice();	
	CameraDeviceInfo GetCameraDeviceInfo(); 
	cv::Mat GetFrameData();
	int CameraInputInit(const PreviewCameraInfo& cInfo);
	int DestoryInputParam();
};

#endif