#ifndef _OILPAINTING_H
#define _OILPAINTING_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

// �ͻ���������Ҫ�õ��Ĳ���
struct OilPaintParam {
	int brushSize;		// ��ˢ�뾶
	int coarseness;		// �ֲڶ�
	int arrayLen;		// �����С
	int *countIntensity;// ͳ��ǿ������
	int *redAverage;	// һ�����������Ч�ĺ�ɫ��ֵ����
	int *greenAverage;  // һ�����������Ч����ɫ��ֵ����
	int *blueAverage;	// һ�����������Ч����ɫ��ֵ����
	cv::Mat gray;		// ԭͼ�ĻҶ�ͼ������ͳ��ǿ�ȷֲ�
	cv::Mat dst;		// ���յõ����ͻ�ͼ��
};

class OilPainting
{
public:
	OilPainting();
	~OilPainting();
	void OilPaintProcess(cv::Mat& img);	// �ͻ�Ч��������

	void FindMostNumIndex(
		int* countArray,
		const int& dataSize,
		uchar& maxIndex,
		int& maxNum);					// ���������ҵ���������������

	void XMovePositive(
		OilPaintParam& opp,
		int nY,
		int nX,
		const cv::Mat& img);			// �ͻ�ͼ�����У���ģ�������ƶ�ʱ�Ĵ�����

	void XMoveNegative(
		OilPaintParam& opp,
		int nY,
		int nX,
		const cv::Mat& img);			// �ͻ�ͼ�����У���ģ�������ƶ�ʱ�Ĵ�����

	void YLeftDown(
		OilPaintParam& opp,
		int nY,
		int nXFirst,
		const cv::Mat& img);			// �ͻ�ͼ�����У���ģ������������ƶ�ʱ�Ĵ�����

	void YRightDown(
		OilPaintParam& opp,
		int nY,
		int nXFirst,
		const cv::Mat& img);			// �ͻ�ͼ�����У���ģ�����ұ������ƶ�ʱ�Ĵ�����

};

#endif