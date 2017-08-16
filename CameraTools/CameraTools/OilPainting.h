#ifndef _OILPAINTING_H
#define _OILPAINTING_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

// 油画处理中需要用到的参数
struct OilPaintParam {
	int brushSize;		// 画刷半径
	int coarseness;		// 粗糙度
	int arrayLen;		// 数组大小
	int *countIntensity;// 统计强度数组
	int *redAverage;	// 一个处理块中有效的红色均值数组
	int *greenAverage;  // 一个处理块中有效的绿色均值数组
	int *blueAverage;	// 一个处理块中有效的蓝色均值数组
	cv::Mat gray;		// 原图的灰度图像，用于统计强度分布
	cv::Mat dst;		// 最终得到的油画图像
};

class OilPainting
{
public:
	OilPainting();
	~OilPainting();
	void OilPaintProcess(cv::Mat& img);	// 油画效果处理函数

	void FindMostNumIndex(
		int* countArray,
		const int& dataSize,
		uchar& maxIndex,
		int& maxNum);					// 在数组中找到最大的数即其索引

	void XMovePositive(
		OilPaintParam& opp,
		int nY,
		int nX,
		const cv::Mat& img);			// 油画图像处理中，当模板向右移动时的处理函数

	void XMoveNegative(
		OilPaintParam& opp,
		int nY,
		int nX,
		const cv::Mat& img);			// 油画图像处理中，当模板向左移动时的处理函数

	void YLeftDown(
		OilPaintParam& opp,
		int nY,
		int nXFirst,
		const cv::Mat& img);			// 油画图像处理中，当模板载左边向下移动时的处理函数

	void YRightDown(
		OilPaintParam& opp,
		int nY,
		int nXFirst,
		const cv::Mat& img);			// 油画图像处理中，当模板载右边向下移动时的处理函数

};

#endif