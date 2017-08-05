#include "CameraTools.h"
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qfiledialog.h>
#include <dbt.h>

#pragma comment(lib, "FreeImage.lib")


CameraTools::CameraTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.horizontalSlider_transparency->setMinimum(0);
	ui.horizontalSlider_transparency->setMaximum(100);
	ui.horizontalSlider_transparency->setValue(0);
	b_camera_state = false;
	b_grab = false;
	DetectCamera();
}

void CameraTools::DetectCamera()
{
	b_camera_state = true;
	camera_device_list.clear();
	ui.comboBox_Camera->clear();
	CameraConfig cc;
	camera_device_list = cc.ListCameraDevice();

	if (camera_device_list.size() != 0) {
		for (int i = 0; i < camera_device_list.size(); ++i)
		{
			ui.comboBox_Camera->addItems((QStringList)camera_device_list[i].friend_name.c_str());
		}
		ui.comboBox_Camera->setCurrentIndex(0);
		ShowCameraInfo(camera_device_list, 0);
	}
	b_camera_state = false;
}

void CameraTools::ShowCameraInfo(std::vector<CameraDeviceInfo>& camera_info, int index)
{
	ui.comboBox_DataFps->clear();
	ui.comboBox_DataType->clear();
	ui.comboBox_DataResolution->clear();
	ui.comboBox_DataBit->clear();
	std::set<std::string>::iterator it;
	for (it = camera_info[index].data_type.begin(); 
		it != camera_info[index].data_type.end(); it++) {
		QString qs = QString::fromStdString(*it);
		ui.comboBox_DataType->addItems((QStringList)qs);
	}

	for (it = camera_info[index].data_resolution.begin(); 
		it != camera_info[index].data_resolution.end(); it++) {
		QString qs = QString::fromStdString(*it);
		ui.comboBox_DataResolution->addItems((QStringList)qs);
	}

	for (it = camera_info[index].data_bit.begin(); 
		it != camera_info[index].data_bit.end(); it++) {
		QString qs = QString::fromStdString(*it);
		ui.comboBox_DataBit->addItems((QStringList)qs);
	}

	for (it = camera_info[index].data_fps.begin(); 
		it != camera_info[index].data_fps.end(); it++) {
		QString qs = QString::fromStdString(*it);
		ui.comboBox_DataFps->addItems((QStringList)qs);
	}
}

cv::Mat CameraTools::BitToMat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif)
{
	if (NULL == fiBmp || FIF_GIF != fif)
		return cv::Mat();
	int width = FreeImage_GetWidth(fiBmp);
	int height = FreeImage_GetHeight(fiBmp);

	RGBQUAD* pixels = new RGBQUAD;
	cv::Mat img = cv::Mat::zeros(height, width, CV_8UC3);
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			FreeImage_GetPixelColor(fiBmp, j, i, pixels);
			img.at<uchar>(height - 1 - i, 3 * j) = pixels->rgbBlue;
			img.at<uchar>(height - 1 - i, 3 * j + 1) = pixels->rgbGreen;
			img.at<uchar>(height - 1 - i, 3 * j + 2) = pixels->rgbRed;
		}
	}

	return img;
}

int CameraTools::GifToMat(std::vector<cv::Mat>& gifImgs, const char* filename)
{
	FreeImage_Initialise(); 
	FREE_IMAGE_FORMAT fif = FIF_GIF;
	FIBITMAP* fiBmp1 = FreeImage_Load(fif, filename, GIF_DEFAULT);
	FIMULTIBITMAP * fiBmp = FreeImage_OpenMultiBitmap(fif, filename, 0, 1, 0, GIF_PLAYBACK);

	int num = FreeImage_GetPageCount(fiBmp);
	for (int i = 0; i < num; i++)
	{
		FIBITMAP *mfibmp = FreeImage_LockPage(fiBmp, i);
		if (!mfibmp) {
			continue;
		}
		cv::Mat dst = BitToMat(mfibmp, fif);
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

	if (NULL != fiBmp)
		FreeImage_CloseMultiBitmap(fiBmp);
	FreeImage_DeInitialise();
	return 0;
}

bool CameraTools::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	Q_UNUSED(eventType);
	MSG* msg = reinterpret_cast<MSG*>(message);
	int msgType = msg->message;
	if (msgType == WM_DEVICECHANGE) {
		switch (msg->wParam)
		{
		case DBT_DEVNODES_CHANGED:
			DetectCamera();
			break;
		default:
			break;
		}
	}
	return false;
}

void CameraTools::paint_img(QImage src_img)
{
	ui.label_show_picture->resize(src_img.size());
	ui.label_show_picture->setPixmap(QPixmap::fromImage(src_img));
	if (b_grab) {
		b_grab = false;
		QDateTime time = QDateTime::currentDateTime();
		QString str = time.toString("yyyyMMddhhmmsszzz") + ".jpg";
		src_img.save(str);
	}
}

void CameraTools::button_startshow_click()
{
	QString str = ui.pushButton_StartShow->text();
	if (str == QStringLiteral("开始显示")) {
		ds1 = new DirectShowTools();
		connect(ds1, SIGNAL(send_image_data(QImage)), this,
			SLOT(paint_img(QImage)), Qt::QueuedConnection);
		ds1->start();
		ui.pushButton_StartShow->setText(QStringLiteral("停止显示"));
	}
	else {
		connect(this, SIGNAL(ThreadStop()), ds1,
			    SLOT(ThreadStopFunc()), Qt::QueuedConnection);
		emit ThreadStop();
		ui.pushButton_StartShow->setText(QStringLiteral("开始显示"));
	}
}

void CameraTools::button_grab_click()
{
	b_grab = true;
//	CameraConfig cc;
//	cc.ConfigCamera();
}

void CameraTools::button_addmask_click()
{
	QString str = ui.pushButton_addmask->text();
	if (str == QStringLiteral("添加水印")) {
		QString path = QFileDialog::getOpenFileName(this,
			QStringLiteral("选择添加的水印图像"),
			".",
			tr("Image Files(*.gif *.jpg *.png)"));
		if (path.length() != 0) {
			char* filepath;
			QByteArray tmp = path.toLatin1();
			filepath = tmp.data();

			if (path.endsWith(".gif")) {
				GifToMat(ds1->m_WaterMaskGifImg, filepath);
				ds1->m_MaskFlag = 2;
			}
			else {
				ds1->m_WaterMaskImg = cv::imread(filepath);
				ds1->m_MaskImg = cv::imread(filepath, 0);
				ds1->m_MaskFlag = 1;
			}
		}
		else {
			ds1->m_MaskFlag = 0;
		}
		ui.pushButton_addmask->setText(QStringLiteral("删除水印"));
	}
	else {
		ds1->m_MaskFlag = 0;
		ui.pushButton_addmask->setText(QStringLiteral("添加水印"));
	}
}

void CameraTools::combobox_camera_change()
{
	int i = ui.comboBox_Camera->currentIndex();
	if (!b_camera_state) {
		ShowCameraInfo(camera_device_list, i);
	}
}

void CameraTools::slider_value_change()
{
	int i = ui.horizontalSlider_transparency->value();
	ds1->m_Transparency = i;
}