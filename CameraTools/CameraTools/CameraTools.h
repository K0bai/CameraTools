#ifndef _CAMERATOOLS_H
#define _CAMERATOOLS_H

#include <QtWidgets/QMainWindow>
#include "ui_CameraTools.h"
#include <QMouseEvent>

#include "DataProcess.h"
#include "CameraDetectTools.h"
#include "DirectShowTools.h"
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

// 是否移动水印
#define CT_MOVEWATERMASK_NO 0
#define CT_MOVEWATERMASK_YES 1

class CameraTools : public QMainWindow
{
	Q_OBJECT

public:
	CameraTools(QWidget *parent = Q_NULLPTR);
	PreviewCameraInfo GetCameraParam();			 // 得到从界面设置的摄像头参数
	void DetectCamera();						 // 用于检测系统中可以使用的摄像头
	void WriteRecord(QString msg);				 // 输出日志记录
	void UpdateControl(int flag);				 // 更新界面控件

	void ShowCameraInfo(
		const std::vector<CameraDeviceInfo>& camera_info,
		const int& index);						 // 用于展示检测出的摄像头的具体数据

	void StartPreview(DataProcess* pDP);		 // 开始预览处理函数，将检测工具类指针传入

	void GetResolutionFromString(
		const std::string &res,
		int &width,
		int &height);							 // 将分辨率转换成int型的长和宽

	void BitToMat(
		FIBITMAP* fiBmp, 
		const FREE_IMAGE_FORMAT &fif,
		cv::Mat& gifImg, 
		cv::Mat& maskImg);						 // 函数用于将FIBITMAP*格式的数据转换为cv::Mat的数据

	int GifToMat(
		std::vector<cv::Mat>& gifImgs, 
		std::vector<cv::Mat>& maskImgs, 
		std::string filename);					 // 用于读取GIF格式图像并将数据内容转换为cv::Mat

	int IsWaterMaskSizeOk(
		const int& height, 
		const int& width);						 // 用于判断添加水印大小是否合法

protected:
	bool nativeEvent(const QByteArray &eventType,
					 void *message, 
					 long *result);				 // 用于硬件热拔插事件处理
	void mouseMoveEvent(QMouseEvent *event);	 // 鼠标移动事件处理
	void mousePressEvent(QMouseEvent *event);	 // 鼠标点击事件处理
private:
	Ui::CameraToolsClass ui;
	
	CameraDetectTools *m_pCDT;
	DataProcess *m_dp;

	QPoint m_startPoint;
	QPoint m_pointOffset;

	bool b_cameraInitState;
	int m_detectTools;
	int m_captureState;
	int m_isMoveWaterMask;
	std::vector<CameraDeviceInfo> m_cameraList;

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