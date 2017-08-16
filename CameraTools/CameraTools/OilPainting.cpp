#include "OilPainting.h"



OilPainting::OilPainting()
{
}


OilPainting::~OilPainting()
{
}


/*
* 找到数组中的最大值及其索引
* 参数：
*		countArray: 目标数组
*		dataSize: 数组大小
*		maxIndex: 最大值的索引
*		maxNum: 最大值
*/
void OilPainting::FindMostNumIndex(
	int* countArray,
	const int& dataSize,
	uchar& maxIndex,
	int& maxNum)
{
	maxNum = countArray[0];
	for (int i = 1; i < dataSize; i++) {
		if (countArray[i] > maxNum) {
			maxIndex = (uchar)i;
			maxNum = countArray[i];
		}
	}
}

/*
* 模板向右移动时的处理函数
* 参数：
*		opp: OilPaintParam结构体
*		nY: 当前图像高度索引值
*		nX: 当前图像长度索引值
*		img: 原始待处理的图像，用于提供模板中的R，G，B值
*/
void OilPainting::XMovePositive(
	OilPaintParam& opp,
	int nY,
	int nX,
	const cv::Mat& img)
{
	for (int i = nY - opp.brushSize; i <= nY + opp.brushSize; ++i) {
		uchar nlast =
			static_cast<uchar>(opp.coarseness*opp.gray.at<uchar>(i, nX - opp.brushSize - 1) / 255.0);
		uchar ncurr =
			static_cast<uchar>(opp.coarseness*opp.gray.at<uchar>(i, nX + opp.brushSize) / 255.0);

		opp.countIntensity[nlast]--;
		opp.countIntensity[ncurr]++;

		opp.redAverage[nlast] -= img.at<cv::Vec3b>(i, nX - opp.brushSize - 1)[2];
		opp.greenAverage[nlast] -= img.at<cv::Vec3b>(i, nX - opp.brushSize - 1)[1];
		opp.blueAverage[nlast] -= img.at<cv::Vec3b>(i, nX - opp.brushSize - 1)[0];

		opp.redAverage[ncurr] += img.at<cv::Vec3b>(i, nX + opp.brushSize)[2];
		opp.greenAverage[ncurr] += img.at<cv::Vec3b>(i, nX + opp.brushSize)[1];
		opp.blueAverage[ncurr] += img.at<cv::Vec3b>(i, nX + opp.brushSize)[0];
	}

	uchar ch = 0;
	int maxNum = 0;
	FindMostNumIndex(opp.countIntensity, opp.arrayLen, ch, maxNum);

	opp.dst.at<cv::Vec3b>(nY, nX)[2] =
		static_cast<uchar>(opp.redAverage[ch] / static_cast<float>(maxNum));
	opp.dst.at<cv::Vec3b>(nY, nX)[1] =
		static_cast<uchar>(opp.greenAverage[ch] / static_cast<float>(maxNum));
	opp.dst.at<cv::Vec3b>(nY, nX)[0] =
		static_cast<uchar>(opp.blueAverage[ch] / static_cast<float>(maxNum));
}

/*
* 模板向左移动时的处理函数
* 参数：
*		opp: OilPaintParam结构体
*		nY: 当前图像高度索引值
*		nX: 当前图像长度索引值
*		img: 原始待处理的图像，用于提供模板中的R，G，B值
*/
void OilPainting::XMoveNegative(
	OilPaintParam& opp,
	int nY,
	int nX,
	const cv::Mat& img)
{
	for (int i = nY - opp.brushSize; i <= nY + opp.brushSize; ++i) {
		uchar ncurr =
			static_cast<uchar>(opp.coarseness*opp.gray.at<uchar>(i, nX - opp.brushSize) / 255.0);
		uchar nlast =
			static_cast<uchar>(opp.coarseness*opp.gray.at<uchar>(i, nX + opp.brushSize + 1) / 255.0);

		opp.countIntensity[nlast]--;
		opp.countIntensity[ncurr]++;

		opp.redAverage[nlast] -= img.at<cv::Vec3b>(i, nX + opp.brushSize + 1)[2];
		opp.greenAverage[nlast] -= img.at<cv::Vec3b>(i, nX + opp.brushSize + 1)[1];
		opp.blueAverage[nlast] -= img.at<cv::Vec3b>(i, nX + opp.brushSize + 1)[0];

		opp.redAverage[ncurr] += img.at<cv::Vec3b>(i, nX - opp.brushSize)[2];
		opp.greenAverage[ncurr] += img.at<cv::Vec3b>(i, nX - opp.brushSize)[1];
		opp.blueAverage[ncurr] += img.at<cv::Vec3b>(i, nX - opp.brushSize)[0];
	}

	uchar ch = 0;
	int maxNum = 0;
	FindMostNumIndex(opp.countIntensity, opp.arrayLen, ch, maxNum);

	opp.dst.at<cv::Vec3b>(nY, nX)[2] =
		static_cast<uchar>(opp.redAverage[ch] / static_cast<float>(maxNum));
	opp.dst.at<cv::Vec3b>(nY, nX)[1] =
		static_cast<uchar>(opp.greenAverage[ch] / static_cast<float>(maxNum));
	opp.dst.at<cv::Vec3b>(nY, nX)[0] =
		static_cast<uchar>(opp.blueAverage[ch] / static_cast<float>(maxNum));

}

/*
* 模板在左边向下移动时的处理函数
* 参数：
*		opp: OilPaintParam结构体
*		nY: 当前图像高度索引值
*		nXFirst: 当前图像长度索引值
*		img: 原始待处理的图像，用于提供模板中的R，G，B值
*/
void OilPainting::YLeftDown(
	OilPaintParam& opp,
	int nY,
	int nXFirst,
	const cv::Mat& img)
{
	if (nY != opp.brushSize) {
		for (int i = nXFirst - opp.brushSize; i <= nXFirst + opp.brushSize; ++i) {
			uchar nlast =
				static_cast<uchar>(opp.coarseness*opp.gray.at<uchar>(nY - opp.brushSize - 1, i) / 255.0);
			uchar ncurr =
				static_cast<uchar>(opp.coarseness*opp.gray.at<uchar>(nY + opp.brushSize, i) / 255.0);

			opp.countIntensity[nlast]--;
			opp.countIntensity[ncurr]++;

			opp.redAverage[nlast] -= img.at<cv::Vec3b>(nY - opp.brushSize - 1, i)[2];
			opp.greenAverage[nlast] -= img.at<cv::Vec3b>(nY - opp.brushSize - 1, i)[1];
			opp.blueAverage[nlast] -= img.at<cv::Vec3b>(nY - opp.brushSize - 1, i)[0];

			opp.redAverage[ncurr] += img.at<cv::Vec3b>(nY + opp.brushSize, i)[2];
			opp.greenAverage[ncurr] += img.at<cv::Vec3b>(nY + opp.brushSize, i)[1];
			opp.blueAverage[ncurr] += img.at<cv::Vec3b>(nY + opp.brushSize, i)[0];
		}

		uchar ch = 0;
		int maxNum = 0;
		FindMostNumIndex(opp.countIntensity, opp.arrayLen, ch, maxNum);

		opp.dst.at<cv::Vec3b>(nY, nXFirst)[2] =
			static_cast<uchar>(opp.redAverage[ch] / static_cast<float>(maxNum));
		opp.dst.at<cv::Vec3b>(nY, nXFirst)[1] =
			static_cast<uchar>(opp.greenAverage[ch] / static_cast<float>(maxNum));
		opp.dst.at<cv::Vec3b>(nY, nXFirst)[0] =
			static_cast<uchar>(opp.blueAverage[ch] / static_cast<float>(maxNum));

	}
}

/*
* 模板在右边向下移动时的处理函数
* 参数：
*		opp: OilPaintParam结构体
*		nY: 当前图像高度索引值
*		nXFirst: 当前图像长度索引值
*		img: 原始待处理的图像，用于提供模板中的R，G，B值
*/
void OilPainting::YRightDown(
	OilPaintParam& opp,
	int nY,
	int nXFirst,
	const cv::Mat& img)
{
	for (int i = nXFirst - opp.brushSize - 1; i < nXFirst + opp.brushSize; ++i) {
		uchar nlast = static_cast<uchar>(opp.coarseness*opp.gray.at<uchar>(nY - opp.brushSize - 1, i) / 255.0);
		uchar ncurr = static_cast<uchar>(opp.coarseness*opp.gray.at<uchar>(nY + opp.brushSize, i) / 255.0);

		opp.countIntensity[nlast]--;
		opp.countIntensity[ncurr]++;

		opp.redAverage[nlast] -= img.at<cv::Vec3b>(nY - opp.brushSize - 1, i)[2];
		opp.greenAverage[nlast] -= img.at<cv::Vec3b>(nY - opp.brushSize - 1, i)[1];
		opp.blueAverage[nlast] -= img.at<cv::Vec3b>(nY - opp.brushSize - 1, i)[0];

		opp.redAverage[ncurr] += img.at<cv::Vec3b>(nY + opp.brushSize, i)[2];
		opp.greenAverage[ncurr] += img.at<cv::Vec3b>(nY + opp.brushSize, i)[1];
		opp.blueAverage[ncurr] += img.at<cv::Vec3b>(nY + opp.brushSize, i)[0];
	}

	uchar ch = 0;
	int maxNum = 0;
	FindMostNumIndex(opp.countIntensity, opp.arrayLen, ch, maxNum);

	opp.dst.at<cv::Vec3b>(nY, nXFirst - 1)[2] =
		static_cast<uchar>(opp.redAverage[ch] / static_cast<float>(maxNum));
	opp.dst.at<cv::Vec3b>(nY, nXFirst - 1)[1] =
		static_cast<uchar>(opp.greenAverage[ch] / static_cast<float>(maxNum));
	opp.dst.at<cv::Vec3b>(nY, nXFirst - 1)[0] =
		static_cast<uchar>(opp.blueAverage[ch] / static_cast<float>(maxNum));
}


/*
* 油画处理函数
* 参数：
*		img: 原始待处理的图像
*/
void OilPainting::OilPaintProcess(cv::Mat & img)
{
	OilPaintParam opp;

	opp.brushSize = 3;
	opp.coarseness = 110;
	opp.arrayLen = opp.coarseness + 1;
	opp.countIntensity = new int[opp.arrayLen];
	opp.redAverage = new int[opp.arrayLen];
	opp.greenAverage = new int[opp.arrayLen];
	opp.blueAverage = new int[opp.arrayLen];
	cvtColor(img, opp.gray, cv::COLOR_BGR2GRAY);
	opp.dst = cv::Mat::zeros(img.size(), img.type());
	for (int i = 0; i < opp.arrayLen; i++) {
		opp.countIntensity[i] = 0;
		opp.redAverage[i] = 0;
		opp.greenAverage[i] = 0;
		opp.blueAverage[i] = 0;
	}

	int width = img.cols;
	int height = img.rows;

	// 初始化
	for (int j = 0; j < 2 * opp.brushSize + 1; j++) {
		for (int i = 0; i < 2 * opp.brushSize + 1; i++) {
			uchar intensity = static_cast<uchar>(opp.coarseness*opp.gray.at<uchar>(j, i) / 255.0);
			opp.countIntensity[intensity]++;
			opp.redAverage[intensity] += img.at<cv::Vec3b>(j, i)[2];
			opp.greenAverage[intensity] += img.at<cv::Vec3b>(j, i)[1];
			opp.blueAverage[intensity] += img.at<cv::Vec3b>(j, i)[0];
		}
	}

	uchar ch = 0;
	int maxNum = 0;
	FindMostNumIndex(opp.countIntensity, opp.arrayLen, ch, maxNum);

	opp.dst.at<cv::Vec3b>(opp.brushSize, opp.brushSize)[2] =
		static_cast<uchar>(opp.redAverage[ch] / static_cast<float>(maxNum));
	opp.dst.at<cv::Vec3b>(opp.brushSize, opp.brushSize)[1] =
		static_cast<uchar>(opp.greenAverage[ch] / static_cast<float>(maxNum));
	opp.dst.at<cv::Vec3b>(opp.brushSize, opp.brushSize)[0] =
		static_cast<uchar>(opp.blueAverage[ch] / static_cast<float>(maxNum));
	// 结束初始化

	//开始循环处理，暂时不考虑边界条件
	for (int nY = opp.brushSize; nY < height - opp.brushSize; nY++) {
		int nXFirst;
		int nXEnd;

		// 反向走
		if ((nY - opp.brushSize) % 2 == 1) {
			nXFirst = width - opp.brushSize;
			nXEnd = opp.brushSize;
			YRightDown(opp, nY, nXFirst, img);

			for (int nX = nXFirst - 1 - 1; nX >= nXEnd; nX--) {
				XMoveNegative(opp, nY, nX, img);
			}
		}
		else { // 正向走
			nXFirst = opp.brushSize;
			nXEnd = width - opp.brushSize;
			YLeftDown(opp, nY, nXFirst, img);

			for (int nX = nXFirst + 1; nX < nXEnd; nX++) {
				XMovePositive(opp, nY, nX, img);
			}
		}
	}
	opp.dst.copyTo(img);

	delete[] opp.countIntensity;
	delete[] opp.redAverage;
	delete[] opp.greenAverage;
	delete[] opp.blueAverage;
	opp.gray.release();
	opp.dst.release();
}
