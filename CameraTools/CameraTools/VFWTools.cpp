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
		_T("abcd"),         // ��������  
		WS_POPUP,           // Windows���ڵ�����  
		1, 1, 10, 10,       // ���ڵ�λ�úʹ�С  
		0,                  // ������  
		0                   // ������ʹ��0, ����Ƿ�0, �ʹ���ʧ��, ��֪Ϊʲô.  
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
