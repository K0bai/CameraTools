#ifndef _CAMERADETECTTOOLS_H
#define _CAMERADETECTTOOLS_H

#include <string>
#include <vector>
#include <set>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

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

//  ������Ҫ��ȡ������ͷ�豸��صĲ���
struct CameraDeviceInfo {
	std::string friend_name;								 // �豸����
	std::set<std::string> data_type;						 // �豸��֧�ֵ������������
	std::set<std::string, ClassCompareRes> data_resolution;  // �豸��֧�ֵ�����ֱ���
	std::set<std::string, ClassCompareOth> data_fps;		 // �豸��֧�ֵ����֡��
};

// ����ͷ����ʱ�Ĳ���
struct PreviewCameraInfo {
	std::string name;			// ����ͷ����
	std::string type;			// ����ͷ����ʱ�������������
	std::string resolution;		// ����ͷ����ʱ�ķֱ���
	std::string fps;			// ����ͷ����ʱ��֡��
};

// �������ͷ�Լ���ȡͼ��������
class CameraDetectTools
{
public:
	virtual std::vector<CameraDeviceInfo> ListCameraDevice() = 0;		// ö�ٵ�ǰϵͳ���õ�����ͷ
	virtual CameraDeviceInfo GetCameraDeviceInfo() = 0;					// ��ȡϵͳ����ͷ�豸����ز���
	virtual cv::Mat GetFrameData() = 0;									// �õ�����ͷһ֡��ͼ������
	virtual int CameraInputInit(const PreviewCameraInfo& cInfo) = 0;	// ��������ͷ����ʼ��
	virtual int DestoryInputParam() = 0;								// �ͷ�����ͷ��صĲ���		
};

#endif