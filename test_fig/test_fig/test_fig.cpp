// test_fig.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "FreeImage.h"
#include "cv.h"
#include "opencv2\highgui\highgui.hpp"

using namespace cv;

#pragma comment(lib, "FreeImage.lib")

Mat bitMap2Mat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif)
{
	if (NULL == fiBmp || FIF_GIF != fif)
		return Mat();
	int width = FreeImage_GetWidth(fiBmp);
	int height = FreeImage_GetHeight(fiBmp);

	RGBQUAD* pixels = new RGBQUAD;
	Mat img = Mat::zeros(height, width, CV_8UC3);
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			FreeImage_GetPixelColor(fiBmp, j, i, pixels);
			img.at<uchar>(height-1-i, 3*j) = pixels->rgbBlue;
			img.at<uchar>(height-1-i, 3*j+1) = pixels->rgbGreen;
			img.at<uchar>(height-1-i, 3*j+2) = pixels->rgbRed;
		}
	}

	return img;
}

int gif2Mat(vector<Mat>& gifImgs)
{
	
	char* filename = "xiaoxin.gif";
	FreeImage_Initialise();         //load the FreeImage function lib 
	FREE_IMAGE_FORMAT fif = FIF_GIF;
	FIBITMAP* fiBmp1 = FreeImage_Load(fif, filename, GIF_DEFAULT);
	FIMULTIBITMAP * fiBmp = FreeImage_OpenMultiBitmap(fif, filename, 0, 1, 0, GIF_PLAYBACK);

	int num = FreeImage_GetPageCount(fiBmp);
	for (int i = 0; i < num; i++)
	{
		FIBITMAP *mfibmp = FreeImage_LockPage(fiBmp, i);
		if (mfibmp)
		{
			Mat dst = bitMap2Mat(mfibmp, fif);
			FreeImage_UnlockPage(fiBmp, mfibmp, false);
			if (dst.empty())
			{
				if (NULL != fiBmp) FreeImage_CloseMultiBitmap(fiBmp, GIF_DEFAULT);
				FreeImage_DeInitialise();
				return -1;
			}
			gifImgs.push_back(dst);
			dst.release();
		}
	}

	if (NULL != fiBmp)
		FreeImage_CloseMultiBitmap(fiBmp);
	FreeImage_DeInitialise();

	return 0;
}

int main()
{
	std::vector<Mat> gifMat;
	gif2Mat(gifMat);

	for (int i = 0; i < gifMat.size(); ++i) {
		imwrite(std::to_string(i) + ".jpg", gifMat[i]);
	//	imshow(std::to_string(i), gifMat[i]);
	}

	system("pause");
    return 0;
}

