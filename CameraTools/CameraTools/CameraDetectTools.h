#ifndef _CAMERADETECTTOOLS_H
#define _CAMERADETECTTOOLS_H

#include <string>
#include <vector>
#include <set>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

//  自定义STL中的set容器的排序方式，用于界面顺序输出显示
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

//  软件需要获取的摄像头设备相关的参数
struct CameraDeviceInfo {
	std::string friend_name;								 // 设备名称
	std::set<std::string> data_type;						 // 设备所支持的输出数据类型
	std::set<std::string, ClassCompareRes> data_resolution;  // 设备所支持的输出分辨率
	std::set<std::string, ClassCompareOth> data_bit;		 // 设备所支持的输出数据位数
	std::set<std::string, ClassCompareOth> data_fps;		 // 设备所支持的输出帧率
};

// 摄像头工作时的参数
struct PreviewCameraInfo {
	std::string name;
	std::string type;
	std::string resolution;
	std::string bit;
	std::string fps;
};

// 检测摄像头以及获取图像数据类
class CameraDetectTools
{
public:
	virtual std::vector<CameraDeviceInfo> ListCameraDevice() = 0;		// 枚举当前系统可用的摄像头
	virtual CameraDeviceInfo GetCameraDeviceInfo() = 0;					// 获取系统摄像头设备的相关参数
	virtual cv::Mat GetFrameData() = 0;									// 得到摄像头一帧的图像数据
	virtual int CameraInputInit(const PreviewCameraInfo& cInfo) = 0;	// 配置摄像头及初始化
	virtual int DestoryInputParam() = 0;								// 释放摄像头相关的参数		
};

class CameraDetectToolsInterface
{
public:
	CameraDetectToolsInterface(CameraDetectTools *pCDT) : pCDTI(pCDT) {

	}

	std::vector<CameraDeviceInfo> ListDevice() {
		return pCDTI->ListCameraDevice();
	}

	CameraDeviceInfo GetDeviceInfo() {
		return pCDTI->GetCameraDeviceInfo();
	}

	cv::Mat GetFrameData() {
		return pCDTI->GetFrameData();
	}

	int CameraInit(const PreviewCameraInfo& cInfo) {
		return pCDTI->CameraInputInit(cInfo);
	}

	int DestoryInputParam() {
		return pCDTI->DestoryInputParam();
	}
private:
	CameraDetectTools *pCDTI;
};

#endif