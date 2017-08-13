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



//  自定义STL中的set容器的排序方式，用于界面顺序输出显示
struct ClassCompareRes0 {
	bool operator()(const std::string& lhs, const std::string& rhs) {
		std::string lhs_ = lhs, rhs_ = rhs;
		int index = lhs_.find('x');
		lhs_.erase(index);
		index = rhs_.find('x');
		rhs_.erase(index);
		return std::atoi(lhs_.c_str()) > std::atoi(rhs_.c_str());
	}
};

struct ClassCompareOth0 {
	bool operator()(const std::string& lhs, const std::string& rhs) {
		return std::atoi(lhs.c_str()) > std::atoi(rhs.c_str());
	}
};

//  软件需要获取的摄像头设备相关的参数
struct CameraDeviceInfo0 {
	std::string friend_name;								 // 设备名称
	std::set<std::string> data_type;						 // 设备所支持的输出数据类型
	std::set<std::string, ClassCompareRes0> data_resolution;  // 设备所支持的输出分辨率
	std::set<std::string, ClassCompareOth0> data_bit;		 // 设备所支持的输出数据位数
	std::set<std::string, ClassCompareOth0> data_fps;		 // 设备所支持的输出帧率
};

/**
 * 该类用于获取摄像头设备的相关参数。 
 * 同时该类也可用于设置录像时摄像头的输出参数。
 */
class CameraConfig
{
public:
	CameraConfig();
	~CameraConfig();

public:
	std::vector<CameraDeviceInfo0> ListCameraDevice();		  // 枚举当前系统可用的摄像头
	CameraDeviceInfo0 GetCameraDeviceInfo(IMoniker* pMoniker); // 获取系统摄像头设备的相关参数。
//	void ConfigCamera(IMoniker *pMoniker);

	// 将获取的摄像头的GUID转换成相对应的数据格式
	int GetSubType(GUID guid, char* buffer);
};

#endif