#include "CameraTools.h"
#include <qmessagebox.h>
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
	b_CameraState = false;
	DetectCamera();
}

void CameraTools::DetectCamera()
{
	b_CameraState = true;
	m_CameraList.clear();
	ui.comboBox_Camera->clear();
	CameraConfig cc;
	m_CameraList = cc.ListCameraDevice();

	if (m_CameraList.size() == 0) {
		b_CameraState = false;
		return;
	}
	for (int i = 0; i < m_CameraList.size(); ++i) {
		ui.comboBox_Camera->addItems((QStringList)m_CameraList[i].friend_name.c_str());
	}
	ui.comboBox_Camera->setCurrentIndex(0);
	ShowCameraInfo(m_CameraList, 0);
	b_CameraState = false;
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

PreviewCameraInfo CameraTools::GetCameraParam()
{
	PreviewCameraInfo cInfo;
	cInfo.name = ui.comboBox_Camera->currentText().toStdString();
	cInfo.type = ui.comboBox_DataType->currentText().toStdString();
	cInfo.resolution = ui.comboBox_DataResolution->currentText().toStdString();
	cInfo.fps = ui.comboBox_DataFps->currentText().toStdString();
	cInfo.bit = ui.comboBox_DataBit->currentText().toStdString();
	
	return cInfo;
}

cv::Mat CameraTools::BitToMat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif)
{
	if (NULL == fiBmp || FIF_GIF != fif) {
		return cv::Mat();
	}
	int width = FreeImage_GetWidth(fiBmp);
	int height = FreeImage_GetHeight(fiBmp);

	RGBQUAD* pixels = new RGBQUAD;
	cv::Mat img = cv::Mat::zeros(height, width, CV_8UC3);
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			FreeImage_GetPixelColor(fiBmp, j, i, pixels);
			img.at<uchar>(height-1-i, 3*j) = pixels->rgbBlue;
			img.at<uchar>(height-1-i, 3*j+1) = pixels->rgbGreen;
			img.at<uchar>(height-1-i, 3*j+2) = pixels->rgbRed;
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
	for (int i = 0; i < num; i++) {
		FIBITMAP *mfibmp = FreeImage_LockPage(fiBmp, i);
		if (!mfibmp) {
			continue;
		}
		cv::Mat dst = BitToMat(mfibmp, fif);
		FreeImage_UnlockPage(fiBmp, mfibmp, false);
		if (dst.empty()) {
			if (NULL != fiBmp) {
				FreeImage_CloseMultiBitmap(fiBmp, GIF_DEFAULT);
			}
			FreeImage_DeInitialise();
			return -1;
		}
		gifImgs.push_back(dst);
		dst.release();
	}

	if (NULL != fiBmp) {
		FreeImage_CloseMultiBitmap(fiBmp);
	}
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
}

void CameraTools::button_startshow_click()
{
	QString str = ui.pushButton_StartShow->text();
	if (str == QStringLiteral("¿ªÊ¼Ô¤ÀÀ")) {
		m_Dst = new DirectShowTools();
		connect(m_Dst, SIGNAL(SendImageData(QImage)), this,
			SLOT(paint_img(QImage)), Qt::QueuedConnection);
		PreviewCameraInfo cInfo = GetCameraParam();
		m_Dst->m_CameraName = cInfo.name;
		m_Dst->start();
		ui.pushButton_StartShow->setText(QStringLiteral("Í£Ö¹Ô¤ÀÀ"));
	}
	else {
		str = ui.pushButton_StartCapture->text();
		if (str == QStringLiteral("Í£Ö¹Â¼Ïñ")) {
			QMessageBox::about(NULL, QStringLiteral("ÌáÊ¾"), 
				QStringLiteral("ÇëÏÈÍ£Ö¹Â¼Ïñ"));
			return ;
		}
		connect(this, SIGNAL(ThreadStop()), m_Dst, 
			SLOT(ThreadStopFunc()), Qt::QueuedConnection);
		emit ThreadStop();
		ui.pushButton_StartShow->setText(QStringLiteral("¿ªÊ¼Ô¤ÀÀ"));
	}
}

void CameraTools::button_startcapture_click()
{
	QString str = ui.pushButton_StartCapture->text();
	if (str == QStringLiteral("¿ªÊ¼Â¼Ïñ")) {
		QString file_path = QFileDialog::getExistingDirectory(this,
			QStringLiteral("ÇëÑ¡ÔñÎÄ¼þ±£´æÂ·¾¶"), "./");
		if (file_path.isEmpty()) {
			return;
		}
		else {
			m_Dst->m_SaveVideoPath = file_path.toStdString();
			m_Dst->m_SaveVideoFlag = DST_SAVEVIDEO_INIT;
		}
		ui.pushButton_StartCapture->setText(QStringLiteral("Í£Ö¹Â¼Ïñ"));
	}
	else {
		m_Dst->m_SaveVideoFlag = DST_SAVEVIDEO_END;
		ui.pushButton_StartCapture->setText(QStringLiteral("¿ªÊ¼Â¼Ïñ"));
	}
}


void CameraTools::button_grab_click()
{
	if (m_Dst->m_GrabImgFlag == DST_GRAB_READY) {
		m_Dst->m_GrabImgFlag = DST_GRAB_PROCESSING;
	}
}

void CameraTools::button_addmask_click()
{
	QString str = ui.pushButton_addmask->text();
	if (str == QStringLiteral("Ìí¼ÓË®Ó¡")) {
		QString path = QFileDialog::getOpenFileName(this,
			QStringLiteral("Ñ¡ÔñÌí¼ÓµÄË®Ó¡Í¼Ïñ"),
			".",
			tr("Image Files(*.gif *.jpg *.png)"));
		if (path.length() != 0) {
			char* filepath;
			QByteArray tmp = path.toLatin1();
			filepath = tmp.data();

			if (path.endsWith(".gif")) {
				GifToMat(m_Dst->m_WaterMaskGifImg, filepath);
				m_Dst->m_MaskFlag = DST_WATERMASK_GIF;
			}
			else {
				m_Dst->m_WaterMaskImg = cv::imread(filepath);
				m_Dst->m_MaskImg = cv::imread(filepath, 0);
				m_Dst->m_MaskFlag = DST_WATERMASK_RGB;
			}
		}
		else {
			m_Dst->m_MaskFlag = DST_WATERMASK_NONE;
		}
		ui.pushButton_addmask->setText(QStringLiteral("É¾³ýË®Ó¡"));
	}
	else {
		m_Dst->m_MaskFlag = DST_WATERMASK_NONE;
		ui.pushButton_addmask->setText(QStringLiteral("Ìí¼ÓË®Ó¡"));
	}
}

void CameraTools::combobox_camera_change()
{
	int i = ui.comboBox_Camera->currentIndex();
	if (!b_CameraState) {
		ShowCameraInfo(m_CameraList, i);
	}
}

void CameraTools::slider_value_change()
{
	m_Dst->m_Transparency = ui.horizontalSlider_transparency->value();
}