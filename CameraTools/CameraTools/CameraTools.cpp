#include "CameraTools.h"
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qfiledialog.h>
#include <dbt.h>


CameraTools::CameraTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	b_cameraInitState = false;
	m_captureState = CT_STOPPREVIEW;
	m_detectTools = CT_DETECTTOOLS_DS;

	m_pCDT = new DirectShowTools;

	ui.horizontalSlider_transparency->setMinimum(0);
	ui.horizontalSlider_transparency->setMaximum(100);
	ui.horizontalSlider_transparency->setValue(0);

	ui.comboBox_ImageStyle->addItems((QStringList)QStringLiteral("正常"));
	ui.comboBox_ImageStyle->addItems((QStringList)QStringLiteral("黑白"));
	ui.comboBox_ImageStyle->addItems((QStringList)QStringLiteral("油画"));
	ui.comboBox_ImageStyle->setCurrentIndex(0);

	ui.comboBox_DetectTools->addItems((QStringList)"DirectShow");
	ui.comboBox_DetectTools->addItems((QStringList)"Media Foundation");
	ui.comboBox_DetectTools->setCurrentIndex(0);
	
	DetectCamera();					// 初始化时默认使用DirectShow检测可用摄像头
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
		ui.comboBox_DataFps->setDisabled(true);
		ui.comboBox_DataResolution->setDisabled(true);
		ui.comboBox_DataType->setDisabled(true);
		ui.comboBox_DetectTools->setDisabled(true);

		ui.pushButton_addmask->setEnabled(true);
		ui.pushButton_Grab->setEnabled(true);
		ui.pushButton_StartCapture->setEnabled(true);

		ui.horizontalSlider_transparency->setEnabled(true);
		ui.horizontalSlider_transparency->setVisible(false);
	}
	else if (flag == CT_STOPPREVIEW) {
		ui.comboBox_Camera->setEnabled(true);
		ui.comboBox_DataFps->setEnabled(true);
		ui.comboBox_DataResolution->setEnabled(true);
		ui.comboBox_DataType->setEnabled(true);
		ui.comboBox_DetectTools->setEnabled(true);

		ui.pushButton_addmask->setDisabled(true);
		ui.pushButton_Grab->setDisabled(true);
		ui.pushButton_StartCapture->setDisabled(true);

		ui.horizontalSlider_transparency->setDisabled(true);
		ui.horizontalSlider_transparency->setVisible(false);
	}
}

void CameraTools::DetectCamera()
{
	b_cameraInitState = true;
	m_cameraList.clear();
	ui.comboBox_Camera->clear();

	m_cameraList = m_pCDT->ListCameraDevice();

	// 如果没检测到当前系统有可用摄像头，直接返回
	if (m_cameraList.size() == 0) {
		b_cameraInitState = false;
		return;
	}

	for (int i = 0; i < m_cameraList.size(); ++i) {
		ui.comboBox_Camera->addItems((QStringList)m_cameraList[i].friend_name.c_str());
	}

	ui.comboBox_Camera->setCurrentIndex(0);
	ShowCameraInfo(m_cameraList, 0);
	b_cameraInitState = false;
}

void CameraTools::ShowCameraInfo(
	const std::vector<CameraDeviceInfo>& camera_info,
	const int& index)
{
	ui.comboBox_DataFps->clear();
	ui.comboBox_DataType->clear();
	ui.comboBox_DataResolution->clear();

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

	for (it = camera_info[index].data_fps.begin(); 
		it != camera_info[index].data_fps.end(); it++) {
		QString qs = QString::fromStdString(*it);
		ui.comboBox_DataFps->addItems((QStringList)qs);
	}
}

void CameraTools::StartPreview(DataProcess * pDP)
{
	PreviewCameraInfo cInfo = GetCameraParam();
	pDP->SetCameraInfo(cInfo);
	pDP->SetDataProcessPtr(m_pCDT);

	ui.openGLWidget_ShowImg->SetDataProcessPtr(m_dp);
	int width = 0;
	int height = 0;
	GetResolutionFromString(cInfo.resolution, width, height);
	ui.openGLWidget_ShowImg->resize(QSize(width, height));

	pDP->start();
}

void CameraTools::GetResolutionFromString(
	const std::string & res, 
	int & width, 
	int & height)
{
	int index = res.find('x');
	if (index == std::string::npos) {
		return;
	}
	width = std::atoi(res.substr(0, index).c_str());
	height = std::atoi(res.substr(index + 1).c_str());
}

PreviewCameraInfo CameraTools::GetCameraParam()
{
	PreviewCameraInfo cInfo;
	cInfo.name = ui.comboBox_Camera->currentText().toStdString();
	cInfo.type = ui.comboBox_DataType->currentText().toStdString();
	cInfo.resolution = ui.comboBox_DataResolution->currentText().toStdString();
	cInfo.fps = ui.comboBox_DataFps->currentText().toStdString();
	
	return cInfo;
}

int CameraTools::IsWaterMaskSizeOk(const int& height, const int& width)
{
	if (height > CT_WATERMASK_MAX_HEIGHT ||
		width > CT_WATERMASK_MIN_WIDTH) {
		return -1;
	}

	return 0;
}

void CameraTools::BitToMat(
	FIBITMAP* fiBmp, 
	const FREE_IMAGE_FORMAT &fif,
	cv::Mat& gifImg, 
	cv::Mat& maskImg)
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

int CameraTools::GifToMat(
	std::vector<cv::Mat>& gifImgs, 
	std::vector<cv::Mat>& maskImgs, 
	std::string filename)
{
	FreeImage_Initialise(); 
	FREE_IMAGE_FORMAT fif = FIF_GIF;
	FIBITMAP* fiBmp1 = FreeImage_Load(fif, filename.c_str(), GIF_DEFAULT);

	FIMULTIBITMAP * fiBmp = FreeImage_OpenMultiBitmap(
								fif, filename.c_str(),
								0,
								1,
								0,
								GIF_PLAYBACK);

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

bool CameraTools::nativeEvent(
	const QByteArray &eventType,
	void *message, 
	long *result)
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

void CameraTools::mouseMoveEvent(QMouseEvent * event)
{
	if (event->buttons() & Qt::LeftButton) {
		if (m_isMoveWaterMask != CT_MOVEWATERMASK_YES) {
			return;
		}

		QRect rect = m_dp->GetWaterMaskRect();
		QPoint curPos = mapFromGlobal(QCursor::pos());
		QPoint imgPos = ui.openGLWidget_ShowImg->pos();

		int xoffset = (curPos.x() - imgPos.x() - m_startPoint.x());
		int yoffset = (curPos.y() - imgPos.y() - m_startPoint.y());
		QRect labelRect = ui.openGLWidget_ShowImg->rect();

		if (xoffset + m_startPoint.x() - m_pointOffset.x() < 0) {
			rect.moveLeft(0);
			m_dp->SetWaterMaskRect(rect);
		}
		else if (xoffset + m_startPoint.x() - m_pointOffset.x()  > 
					labelRect.width() - rect.width()) {
			rect.moveLeft(labelRect.width() - rect.width());
			m_dp->SetWaterMaskRect(rect);
		}
		else {
			rect.moveLeft(xoffset + m_startPoint.x() - m_pointOffset.x());
			m_dp->SetWaterMaskRect(rect);
		}

		if (yoffset + m_startPoint.y() - m_pointOffset.y() < 0) {
			rect.moveTop(0);
			m_dp->SetWaterMaskRect(rect);
		}
		else if (yoffset + m_startPoint.y() - m_pointOffset.y() > 
					labelRect.height() - rect.height()) {
			rect.moveTop(labelRect.height() - rect.height());
			m_dp->SetWaterMaskRect(rect);
		}
		else {
			rect.moveTop(yoffset + m_startPoint.y() - m_pointOffset.y());
			m_dp->SetWaterMaskRect(rect);
		}
	}
}

void CameraTools::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton) {
		QRect rect = m_dp->GetWaterMaskRect();
		QPoint curPos = mapFromGlobal(QCursor::pos());
		QPoint imgPos = ui.openGLWidget_ShowImg->pos();

		m_pointOffset.setX(curPos.x() - imgPos.x() - rect.left());
		m_pointOffset.setY(curPos.y() - imgPos.y() - rect.top());

		int xoff = curPos.x() - imgPos.x();
		int yoff = curPos.y() - imgPos.y();

		if ((xoff >= 0) && (yoff >= 0)
			&& rect.contains(xoff, yoff)) {
			m_startPoint.setX(xoff);
			m_startPoint.setY(yoff) ;
			m_isMoveWaterMask = CT_MOVEWATERMASK_YES;
		}
		else {
			m_isMoveWaterMask = CT_MOVEWATERMASK_NO;
		}
	}
}

void CameraTools::paint_img()
{
	ui.openGLWidget_ShowImg->update();
}

void CameraTools::button_startshow_click()
{
	QString str = ui.pushButton_StartShow->text();
	if (str == QStringLiteral("开始预览")) {
		m_dp = new DataProcess();
		
		connect(m_dp, SIGNAL(SendUpdataImgMsg()), this,
			SLOT(paint_img()), Qt::QueuedConnection);
		connect(m_dp, SIGNAL(SendAbort(int)), this,
			SLOT(get_abort(int)), Qt::QueuedConnection);
		
		StartPreview(m_dp);
		WriteRecord(QStringLiteral("开始预览"));

		m_captureState = CT_STARTPREVIEW;
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

		ui.openGLWidget_ShowImg->SetPaintingFlag(OGLW_PAINTING_NO);
		m_dp->SetThreadStop(true);
		WriteRecord(QStringLiteral("正在停止预览，请稍后……"));

		m_captureState = CT_STOPPREVIEW;
		UpdateControl(CT_STOPPREVIEW);

		ui.pushButton_addmask->setText(QStringLiteral("添加水印"));
		ui.pushButton_StartShow->setText(QStringLiteral("开始预览"));
	}
}

void CameraTools::button_startcapture_click()
{
	QString str = ui.pushButton_StartCapture->text();

	if (str == QStringLiteral("开始录像")) {
		QString filePath = QFileDialog::getExistingDirectory(this,
			QStringLiteral("请选择文件保存路径"), "./");

		if (filePath.isEmpty()) {
			WriteRecord(QStringLiteral("路径为空！"));
			return;
		}

		m_dp->SetSaveVideoPath(filePath.toStdString());
		m_dp->SetSaveVideoFlag(DP_SAVEVIDEO_INIT);
		WriteRecord(QStringLiteral("开始录像"));
		WriteRecord(QStringLiteral("视频存储位置：")+filePath);
		ui.pushButton_StartCapture->setText(QStringLiteral("停止录像"));
	}
	else {
		m_dp->SetSaveVideoFlag(DP_SAVEVIDEO_END);
		WriteRecord(QStringLiteral("停止录像"));
		ui.pushButton_StartCapture->setText(QStringLiteral("开始录像"));
	}
}


void CameraTools::button_grab_click()
{
	if (m_dp->GetGrabImgFlag() != DP_GRAB_READY) {
		return;
	}
	if (m_dp->GetSaveGrabImgPath() == "") {
		QString filePath = QFileDialog::getExistingDirectory(this,
			QStringLiteral("请选择文件保存路径"), "./");

		if (filePath.isEmpty()) {
			WriteRecord(QStringLiteral("路径为空！"));
			return;
		}

		WriteRecord(QStringLiteral("图片存储位置：") + filePath);
		m_dp->SetSaveGrabImgPath(filePath.toStdString());
	}

	WriteRecord(QStringLiteral("抓拍"));
	m_dp->SetGrabImgFlag(DP_GRAB_PROCESSING);
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
			m_dp->SetWaterMaskFlag(DP_WATERMASK_NONE);
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

			QRect rect = QRect(0, 0, tempGif[0].cols, tempGif[0].rows);
			m_dp->SetWaterMaskRect(rect);
			m_dp->SetWaterMaskGifImg(tempGif);
			m_dp->SetMaskGifImg(tempMask);
			m_dp->SetWaterMaskFlag(DP_WATERMASK_GIF);
		}
		else {
			cv::Mat srcImg, maskImg;
			srcImg = cv::imread(filepath);

			if (IsWaterMaskSizeOk(srcImg.rows, srcImg.cols)
				== CT_WATERMASK_ERROR) {
				WriteRecord(QStringLiteral("水印尺寸不符合"));
				return;
			}

			QRect rect = QRect(0, 0, srcImg.cols, srcImg.rows);
			m_dp->SetWaterMaskRect(rect);
			m_dp->SetWaterMaskImg(srcImg);
			maskImg = cv::imread(filepath, 0);
			m_dp->SetMaskImg(maskImg);
			m_dp->SetWaterMaskFlag(DP_WATERMASK_RGB);
		}
		WriteRecord(QStringLiteral("添加水印"));
		ui.horizontalSlider_transparency->setVisible(true);
		ui.pushButton_addmask->setText(QStringLiteral("删除水印"));
	}
	else {
		m_dp->SetWaterMaskFlag(DP_WATERMASK_NONE);
		WriteRecord(QStringLiteral("删除水印"));
		ui.horizontalSlider_transparency->setVisible(false);
		ui.pushButton_addmask->setText(QStringLiteral("添加水印"));
	}
}

void CameraTools::combobox_camera_change()
{
	int i = ui.comboBox_Camera->currentIndex();

	if (!b_cameraInitState) {
		ShowCameraInfo(m_cameraList, i);
	}
}
void CameraTools::combobox_imagestyle_change()
{
	if (m_captureState == CT_STOPPREVIEW) {
		return;
	}

	int index = ui.comboBox_ImageStyle->currentIndex();
	switch (index)
	{
	case 0:
		m_dp->SetImageStyle(DP_IMAGESTYLE_NORMAL);
		WriteRecord(QStringLiteral("选择正常风格"));
		break;
	case 1:
		m_dp->SetImageStyle(DP_IMAGESTYLE_GRAY);
		WriteRecord(QStringLiteral("选择黑白风格"));
		break;
	case 2:
		m_dp->SetImageStyle(DP_IMAGESTYLE_OIL);
		WriteRecord(QStringLiteral("选择油画风格"));
		break;
	}
}

void CameraTools::combobox_detecttools_change()
{
	int index = ui.comboBox_DetectTools->currentIndex();

	if (!m_pCDT) {
		delete[] m_pCDT;
	}

	switch (index)
	{
	case CT_DETECTTOOLS_DS:
		m_pCDT = new DirectShowTools();
		break;
	case CT_DETECTTOOLS_MF:
		m_pCDT = new MediaFoundationTools();
		break;
	default:
		break;
	}

	DetectCamera();
}

void CameraTools::slider_value_change()
{
	m_dp->SetTransparency(ui.horizontalSlider_transparency->value());
}

void CameraTools::get_abort(int ret)
{
	QString str;
	if (ret < 0) {
		WriteRecord(QStringLiteral("获取摄像头数据错误！"));
		str = ui.pushButton_StartCapture->text();
		if (str == QStringLiteral("停止录像")) {
			m_dp->SetSaveVideoFlag(DP_SAVEVIDEO_END);
			WriteRecord(QStringLiteral("停止录像"));
			ui.pushButton_StartCapture->setText(QStringLiteral("开始录像"));
		}
	}

	str = ui.pushButton_addmask->text();

	if (str == QStringLiteral("删除水印")) {
		m_dp->SetWaterMaskFlag(DP_WATERMASK_NONE);
		WriteRecord(QStringLiteral("删除水印"));
		ui.horizontalSlider_transparency->setVisible(false);
		ui.pushButton_addmask->setText(QStringLiteral("添加水印"));
	}

	UpdateControl(CT_STOPPREVIEW);
	ui.pushButton_StartShow->setText(QStringLiteral("开始预览"));
	WriteRecord(QStringLiteral("已停止预览"));
//	delete m_dp;
}

void CameraTools::checkbox_filter()
{
	if (ui.checkBox_Filter->isChecked()) {
		m_dp->SetNoiseOptimizationFlag(DP_NOISEOPTIMIZATION_YES);
		WriteRecord(QStringLiteral("启用噪点优化"));
	}
	else {
		m_dp->SetNoiseOptimizationFlag(DP_NOISEOPTIMIZATION_NO);
		WriteRecord(QStringLiteral("停用噪点优化"));
	}
}
