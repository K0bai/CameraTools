#include "VFWTools.h"



VFWTools::VFWTools()
{
}


VFWTools::~VFWTools()
{
}

std::vector<CameraDeviceInfo> VFWTools::ListCameraDevice()
{
	std::vector<CameraDeviceInfo> cInfo;
/*	TCHAR szDeviceName[80];
	TCHAR szDeviceVersion[80];
	for (int wIndex = 0; wIndex < 10; wIndex++)
	{
		if (capGetDriverDescription(wIndex, szDeviceName,
			sizeof(szDeviceName), szDeviceVersion,
			sizeof(szDeviceVersion)))
		{
			int kk = 0;
	

		}
	}

	HWND m_hWndVideo = capCreateCaptureWindow(
		_T("abcd"),         // 窗口名称  
		WS_POPUP,           // Windows窗口的属性  
		1, 1, 10, 10,       // 窗口的位置和大小  
		0,                  // 父窗口  
		0                   // 我这里使用0, 如果是非0, 就创建失败, 不知为什么.  
	);
	while (FALSE == (capDriverConnect(m_hWndVideo, 0)))
	{
		Sleep(10);
	}*/
	return cInfo;
}

CameraDeviceInfo VFWTools::GetCameraDeviceInfo()
{
	CameraDeviceInfo cInfo;
	return cInfo;
}

cv::Mat VFWTools::GetFrameData()
{
	return cv::Mat();
}

int VFWTools::CameraInputInit(const PreviewCameraInfo& cInfo)
{
	return 0;
}

int VFWTools::DestoryInputParam()
{
	return 0;
}
