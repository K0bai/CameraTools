#include "CameraTools.h"
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qfiledialog.h>
#include <dbt.h>

CameraTools::CameraTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	b_CameraInitState = false;
	m_CaptureState = CT_STOPPREVIEW;
	ui.horizontalSlider_transparency->setMinimum(0);
	ui.horizontalSlider_transparency->setMaximum(100);
	ui.horizontalSlider_transparency->setValue(0);
	ui.comboBox_ImageStyle->addItems((QStringList)QStringLiteral("正常"));
	ui.comboBox_ImageStyle->addItems((QStringList)QStringLiteral("黑白"));
	ui.comboBox_ImageStyle->addItems((QStringList)QStringLiteral("油画"));
	ui.comboBox_ImageStyle->setCurrentIndex(0);
	
	DetectCamera();		// 初始化时直接检测可用摄像头
	UpdateControl(CT_STOPPREVIEW);
}

void CameraTools::WriteRecord(QString msg)
{
	QDateTime time = QDateTime::currentDateTime();
	QString strTime = time.toString("hh:mm:ss  ");
	ui.textEdit_record->append(">> " + strTime + msg);
}

void CameraTools::UpdateControl(int flag)
{
	if (flag == CT_STARTPREVIEW) {
		ui.comboBox_Camera->setDisabled(true);
		ui.comboBox_DataBit->setDisabled(true);
		ui.comboBox_DataFps->setDisabled(true);
		ui.comboBox_DataResolution->setDisabled(true);
		ui.comboBox_DataType->setDisabled(true);

		ui.pushButton_addmask->setEnabled(true);
		ui.pushButton_Grab->setEnabled(true);
		ui.pushButton_StartCapture->setEnabled(true);

		ui.horizontalSlider_transparency->setEnabled(true);
		ui.horizontalSlider_transparency->setVisible(false);
	}
	else if (flag == CT_STOPPREVIEW) {
		ui.comboBox_Camera->setEnabled(true);
		ui.comboBox_DataBit->setEnabled(true);
		ui.comboBox_DataFps->setEnabled(true);
		ui.comboBox_DataResolution->setEnabled(true);
		ui.comboBox_DataType->setEnabled(true);

		ui.pushButton_addmask->setDisabled(true);
		ui.pushButton_Grab->setDisabled(true);
		ui.pushButton_StartCapture->setDisabled(true);

		ui.horizontalSlider_transparency->setDisabled(true);
		ui.horizontalSlider_transparency->setVisible(false);
	}
}

void CameraTools::DetectCamera()
{
	b_CameraInitState = true;
	m_CameraList.clear();
	ui.comboBox_Camera->clear();
	CameraConfig cc;
	m_CameraList = cc.ListCameraDevice();

	if (m_CameraList.size() == 0) {
		b_CameraInitState = false;
		return;
	}

	for (int i = 0; i < m_CameraList.size(); ++i) {
		ui.comboBox_Camera->addItems((QStringList)m_CameraList[i].friend_name.c_str());
	}

	ui.comboBox_Camera->setCurrentIndex(0);
	ShowCameraInfo(m_CameraList, 0);
	b_CameraInitState = false;
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

int CameraTools::IsWaterMaskSizeOk(int height, int width)
{
	if (height > CT_WATERMASK_MAX_HEIGHT ||
		width > CT_WATERMASK_MIN_WIDTH) {
		return -1;
	}
	return 0;
}

void CameraTools::BitToMat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT &fif,
						   cv::Mat& gifImg, cv::Mat& maskImg)
{
	if (NULL == fiBmp || FIF_GIF != fif) {
		return ;
	}
	int width = FreeImage_GetWidth(fiBmp);
	int height = FreeImage_GetHeight(fiBmp);

	RGBQUAD* pixels = new RGBQUAD;
	gifImg = cv::Mat::zeros(height, width, CV_8UC3);
	maskImg = cv::Mat::zeros(height, width, CV_8UC1);
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			FreeImage_GetPixelColor(fiBmp, j, i, pixels);
			gifImg.at<uchar>(height-1-i, 3*j+0) = pixels->rgbBlue;
			gifImg.at<uchar>(height-1-i, 3*j+1) = pixels->rgbGreen;
			gifImg.at<uchar>(height-1-i, 3*j+2) = pixels->rgbRed;
			maskImg.at<uchar>(height-1-i, j) = pixels->rgbReserved;
		}
	}
}

int CameraTools::GifToMat(std::vector<cv::Mat>& gifImgs, std::vector<cv::Mat>& maskImgs, 
						  const char* filename)
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
		cv::Mat gifImg, maskImg;
		BitToMat(mfibmp, fif, gifImg, maskImg);
		FreeImage_UnlockPage(fiBmp, mfibmp, false);
		if (gifImg.empty()) {
			if (NULL != fiBmp) {
				FreeImage_CloseMultiBitmap(fiBmp, GIF_DEFAULT);
			}
			FreeImage_DeInitialise();
			return -1;
		}
		gifImgs.push_back(gifImg);
		maskImgs.push_back(maskImg);
		gifImg.release();
		maskImg.release();
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

void CameraTools::paint_img()
{
	QImage src_img;
	m_Dst->GetImageBuffer(src_img);
	ui.label_show_picture->resize(src_img.size());
	ui.label_show_picture->setPixmap(QPixmap::fromImage(src_img));
}

void CameraTools::button_startshow_click()
{
	QString str = ui.pushButton_StartShow->text();
	if (str == QStringLiteral("开始预览")) {
		m_Dst = new DirectShowTools();
		connect(m_Dst, SIGNAL(SendUpdataImgMsg()), this,
			SLOT(paint_img()), Qt::QueuedConnection);
		connect(m_Dst, SIGNAL(SendAbort(int)), this,
			SLOT(get_abort(int)), Qt::QueuedConnection);
		PreviewCameraInfo cInfo = GetCameraParam();
		m_Dst->SetCameraInfo(cInfo);
		m_Dst->start();
		WriteRecord(QStringLiteral("开始预览"));
		m_CaptureState = CT_STARTPREVIEW;
		UpdateControl(CT_STARTPREVIEW);
		ui.pushButton_StartShow->setText(QStringLiteral("停止预览"));
	}
	else {
		str = ui.pushButton_StartCapture->text();
		if (str == QStringLiteral("停止录像")) {
			QMessageBox::about(NULL, QStringLiteral("提示"), 
				QStringLiteral("请先停止录像"));
			return ;
		}
		m_Dst->SetThreadStop(true);
		WriteRecord(QStringLiteral("正在停止预览，请稍后……"));
		m_CaptureState = CT_STOPPREVIEW;
		UpdateControl(CT_STOPPREVIEW);
		ui.pushButton_addmask->setText(QStringLiteral("添加水印"));
		ui.pushButton_StartShow->setText(QStringLiteral("开始预览"));
	}
}

void CameraTools::button_startcapture_click()
{
	QString str = ui.pushButton_StartCapture->text();
	if (str == QStringLiteral("开始录像")) {
		QString file_path = QFileDialog::getExistingDirectory(this,
			QStringLiteral("请选择文件保存路径"), "./");
		if (file_path.isEmpty()) {
			return;
		}
		else {
			m_Dst->SetSaveVideoPath(file_path.toStdString());
			m_Dst->SetSaveVideoFlag(DST_SAVEVIDEO_INIT);
		}
		WriteRecord(QStringLiteral("开始录像"));
		WriteRecord(QStringLiteral("存储位置：")+file_path);
		ui.pushButton_StartCapture->setText(QStringLiteral("停止录像"));
	}
	else {
		m_Dst->SetSaveVideoFlag(DST_SAVEVIDEO_END);
		WriteRecord(QStringLiteral("停止录像"));
		ui.pushButton_StartCapture->setText(QStringLiteral("开始录像"));
	}
}


void CameraTools::button_grab_click()
{
	if (m_Dst->GetGrabImgFlag() == DST_GRAB_READY) {
		WriteRecord(QStringLiteral("抓拍"));
		m_Dst->SetGrabImgFlag(DST_GRAB_PROCESSING);
	}
}

void CameraTools::button_addmask_click()
{
	QString str = ui.pushButton_addmask->text();
	if (str == QStringLiteral("添加水印")) {
		QString path = QFileDialog::getOpenFileName(this,
			QStringLiteral("选择添加的水印图像"),
			".",
			tr("Image Files(*.gif *.jpg *.png)"));
		if (path.length() == 0) {
			m_Dst->SetWaterMaskFlag(DST_WATERMASK_NONE);
			return;
		}
		char* filepath;
		QByteArray tmp = path.toLatin1();
		filepath = tmp.data();

		if (path.endsWith(".gif")) {
			std::vector<cv::Mat> tempGif, tempMask;
			GifToMat(tempGif, tempMask, filepath);
			if (tempGif.size() != 0 &&
				(IsWaterMaskSizeOk(tempGif[0].rows, tempGif[0].cols)
					== CT_WATERMASK_ERROR)) {
				WriteRecord(QStringLiteral("水印尺寸不符合"));
				return;
			}
			m_Dst->SetWaterMaskGifImg(tempGif);
			m_Dst->SetMaskGifImg(tempMask);
			m_Dst->SetWaterMaskFlag(DST_WATERMASK_GIF);
		}
		else {
			cv::Mat srcImg, maskImg;
			srcImg = cv::imread(filepath);
			if (IsWaterMaskSizeOk(srcImg.rows, srcImg.cols)
				== CT_WATERMASK_ERROR) {
				WriteRecord(QStringLiteral("水印尺寸不符合"));
				return;
			}
			m_Dst->SetWaterMaskImg(srcImg);
			maskImg = cv::imread(filepath, 0);
			m_Dst->SetMaskImg(maskImg);
			m_Dst->SetWaterMaskFlag(DST_WATERMASK_RGB);
		}
		WriteRecord(QStringLiteral("添加水印"));
		ui.horizontalSlider_transparency->setVisible(true);
		ui.pushButton_addmask->setText(QStringLiteral("删除水印"));
	}
	else {
		m_Dst->SetWaterMaskFlag(DST_WATERMASK_NONE);
		WriteRecord(QStringLiteral("删除水印"));
		ui.horizontalSlider_transparency->setVisible(false);
		ui.pushButton_addmask->setText(QStringLiteral("添加水印"));
	}
}

void CameraTools::combobox_camera_change()
{
	int i = ui.comboBox_Camera->currentIndex();
	if (!b_CameraInitState) {
		ShowCameraInfo(m_CameraList, i);
	}
}
void CameraTools::combobox_imagestyle_change()
{
	if (m_CaptureState == CT_STOPPREVIEW) {
		return;
	}
	int index = ui.comboBox_ImageStyle->currentIndex();
	switch (index)
	{
	case 0:
		m_Dst->SetImageStyle(DST_IMAGESTYLE_NORMAL);
		WriteRecord(QStringLiteral("选择正常风格"));
		break;
	case 1:
		m_Dst->SetImageStyle(DST_IMAGESTYLE_GRAY);
		WriteRecord(QStringLiteral("选择黑白风格"));
		break;
	case 2:
		m_Dst->SetImageStyle(DST_IMAGESTYLE_OIL);
		WriteRecord(QStringLiteral("选择油画风格"));
		break;
	}
}

void CameraTools::slider_value_change()
{
	m_Dst->SetTransparency(ui.horizontalSlider_transparency->value());
}

void CameraTools::get_abort(int ret)
{
	QString str;
	if (ret < 0) {
		WriteRecord(QStringLiteral("获取摄像头数据错误！"));
		str = ui.pushButton_StartCapture->text();
		if (str == QStringLiteral("停止录像")) {
			m_Dst->SetSaveVideoFlag(DST_SAVEVIDEO_END);
			WriteRecord(QStringLiteral("停止录像"));
			ui.pushButton_StartCapture->setText(QStringLiteral("开始录像"));
		}
	}
	str = ui.pushButton_addmask->text();
	if (str == QStringLiteral("删除水印")) {
		m_Dst->SetWaterMaskFlag(DST_WATERMASK_NONE);
		WriteRecord(QStringLiteral("删除水印"));
		ui.horizontalSlider_transparency->setVisible(false);
		ui.pushButton_addmask->setText(QStringLiteral("添加水印"));
	}
	UpdateControl(CT_STOPPREVIEW);
	ui.pushButton_StartShow->setText(QStringLiteral("开始预览"));
	WriteRecord(QStringLiteral("已停止预览"));
//	delete m_Dst;
}