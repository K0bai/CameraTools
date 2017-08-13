#ifndef _CAMERATOOLS_H
#define _CAMERATOOLS_H

#include <QtWidgets/QMainWindow>
#include "ui_CameraTools.h"
#include <exception>
#include "DataProcess.h"
#include "CameraDetectTools.h"
#include "DirectShowTools.h"
#include "VFWTools.h"
#include "MediaFoundationTools.h"
#include "FreeImage.h"

// 更新控件相关参数
#define CT_STARTPREVIEW 0
#define CT_STOPPREVIEW 1

// 水印相关参数
#define CT_WATERMASK_ERROR -1
#define CT_WATERMASK_OK 0
#define CT_WATERMASK_MAX_HEIGHT 200
#define CT_WATERMASK_MIN_WIDTH 200

// 摄像头检测工具相关参数
#define  CT_DETECTTOOLS_DS 0
#define  CT_DETECTTOOLS_MF 1
#define  CT_DETECTTOOLS_VFW 2

// 界面是否进行绘图
#define  CT_ISPAINTING_NO 0
#define  CT_ISPAINTING_YES 1


class CameraTools : public QMainWindow
{
	Q_OBJECT

public:
	CameraTools(QWidget *parent = Q_NULLPTR);
	void DetectCamera();	// 用于检测系统中可以使用的摄像头
	void WriteRecord(QString msg);			// 输出日志记录
	void UpdateControl(int flag);			// 更新界面控件
	void ShowCameraInfo(const std::vector<CameraDeviceInfo>& camera_info,
						const int& index);  // 用于展示检测出的摄像头的具体数据
	void StartPreview(DataProcess* pDP);
	PreviewCameraInfo GetCameraParam();		// 得到从界面设置的摄像头参数

	/*
	 * 下面两个函数用于处理GIF图像作为水印的情况。
	 * BitToMat函数用于将FIBITMAP*格式的数据转换为cv::Mat的数据。
	 * GifToMat用于读取GIF格式图像并将数据内容转换为cv::Mat
	 */
	void BitToMat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif,
					 cv::Mat& gifImg, cv::Mat& maskImg);
	int GifToMat(std::vector<cv::Mat>& gifImgs, std::vector<cv::Mat>& maskImgs, 
				 std::string filename);
	int IsWaterMaskSizeOk(const int& height, const int& width);	// 用于判断添加水印大小是否合法

protected:
	bool nativeEvent(const QByteArray &eventType,
					 void *message, long *result);	// 用于硬件热拔插事件处理

private:
	Ui::CameraToolsClass ui;
	DataProcess *m_Dst;
	CameraDetectTools *pCDT_DS;
	CameraDetectTools *pCDT_MF;
	CameraDetectTools *pCDT_VFW;
	CameraDetectToolsInterface* pCDTI_DS;
	CameraDetectToolsInterface* pCDTI_MF;
	CameraDetectToolsInterface* pCDTI_VFW;

	int m_DetectTools;
	bool b_CameraInitState;
	int m_CaptureState;
	int m_IsPainting;
	std::vector<CameraDeviceInfo> m_CameraList;

private slots:
	void button_startshow_click();		// 接收开始预览按钮click消息
	void button_grab_click();			// 接收抓拍按钮click消息
	void button_addmask_click();		// 接收添加水印按钮click消息
	void button_startcapture_click();	// 接收开始录像按钮click消息
	void paint_img();					// 接收绘制图像的消息
	void combobox_camera_change();		// 接收摄像机列表索引变化事件
	void combobox_imagestyle_change();	// 接收图像风格列表索引变化事件
	void combobox_detecttools_change(); // 接收摄像头检测工具列表索引变化事件
	void slider_value_change();			// 接收水印透明度滑块值变化事件
	void get_abort(int msg);			// 接收终止预览的消息
	void checkbox_filter();				// 接收噪点优化的消息
};

#endif