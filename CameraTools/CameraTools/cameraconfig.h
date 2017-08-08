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
#include <dvdmedia.h>
#include <wmsdkidl.h>
#include <OleAuto.h>
#include <objbase.h>



//  �Զ���STL�е�set����������ʽ�����ڽ���˳�������ʾ
struct ClassCompareRes {
	bool operator()(const std::string& lhs, const std::string& rhs) {
		std::string lhs_ = lhs, rhs_ = rhs;
		int index = lhs_.find('x');
		lhs_.erase(index);
		index = rhs_.find('x');
		rhs_.erase(index);
		return std::atoi(lhs_.c_str()) > std::atoi(rhs_.c_str());
	}
};

struct ClassCompareOth {
	bool operator()(const std::string& lhs, const std::string& rhs) {
		return std::atoi(lhs.c_str()) > std::atoi(rhs.c_str());
	}
};

//  �����Ҫ��ȡ������ͷ�豸��صĲ���
struct CameraDeviceInfo {
	std::string friend_name;								 // �豸����
	std::set<std::string> data_type;						 // �豸��֧�ֵ������������
	std::set<std::string, ClassCompareRes> data_resolution;  // �豸��֧�ֵ�����ֱ���
	std::set<std::string, ClassCompareOth> data_bit;		 // �豸��֧�ֵ��������λ��
	std::set<std::string, ClassCompareOth> data_fps;		 // �豸��֧�ֵ����֡��
};

/**
 * �������ڻ�ȡ����ͷ�豸����ز����� 
 * ͬʱ����Ҳ����������¼��ʱ����ͷ�����������
 */
class CameraConfig
{
public:
	CameraConfig();
	~CameraConfig();

public:
	std::vector<CameraDeviceInfo> ListCameraDevice();		  // ö�ٵ�ǰϵͳ���õ�����ͷ
	CameraDeviceInfo GetCameraDeviceInfo(IMoniker* pMoniker); // ��ȡϵͳ����ͷ�豸����ز�����
//	void ConfigCamera(IMoniker *pMoniker);

	// ����ȡ������ͷ��GUIDת�������Ӧ�����ݸ�ʽ
	int GetSubType(GUID guid, char* buffer);
};

#endif