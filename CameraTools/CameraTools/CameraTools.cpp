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
	m_DetectTools = CT_DETECTTOOLS_DS;
	m_IsPainting = CT_ISPAINTING_YES;

	pCDT_DS = new DirectShowTools;
	pCDT_MF = new MediaFoundationTools;
	pCDT_VFW = new VFWTools;
	pCDTI_DS = new CameraDetectToolsInterface(pCDT_DS);
	pCDTI_MF = new CameraDetectToolsInterface(pCDT_MF);
	pCDTI_VFW = new CameraDetectToolsInterface(pCDT_VFW);

	ui.horizontalSlider_transparency->setMinimum(0);
	ui.horizontalSlider_transparency->setMaximum(100);
	ui.horizontalSlider_transparency->setValue(0);

	ui.comboBox_ImageStyle->addItems((QStringList)QStringLiteral("����"));
	ui.comboBox_ImageStyle->addItems((QStringList)QStringLiteral("�ڰ�"));
	ui.comboBox_ImageStyle->addItems((QStringList)QStringLiteral("�ͻ�"));
	ui.comboBox_ImageStyle->setCurrentIndex(0);

	ui.comboBox_DetectTools->addItems((QStringList)"DirectShow");
	ui.comboBox_DetectTools->addItems((QStringList)"Media Foundation");
//	ui.comboBox_DetectTools->addItems((QStringList)"VFW");
	ui.comboBox_DetectTools->setCurrentIndex(0);

	DetectCamera();		// ��ʼ��ʱĬ��ʹ��DirectShow����������ͷ
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
		ui.comboBox_DetectTools->setDisabled(true);

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
	b_CameraInitState = true;
	m_CameraList.clear();
	ui.comboBox_Camera->clear();

	switch (m_DetectTools)
	{
	case CT_DETECTTOOLS_DS:
		m_CameraList = pCDTI_DS->ListDevice();
		break;
	case CT_DETECTTOOLS_MF:
		m_CameraList = pCDTI_MF->ListDevice();
		break;
	case CT_DETECTTOOLS_VFW:
		m_CameraList = pCDTI_VFW->ListDevice();
		break;
	default:
		break;
	}

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

void CameraTools::ShowCameraInfo(const std::vector<CameraDeviceInfo>& camera_info,const int& index)
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

void CameraTools::StartPreview(DataProcess * pDP)
{
	PreviewCameraInfo cInfo = GetCameraParam();
	pDP->SetCameraInfo(cInfo);
	switch (m_DetectTools)
	{
	case CT_DETECTTOOLS_DS:
		pDP->SetCameraToolsInterface(pCDTI_DS);
		break;
	case CT_DETECTTOOLS_MF:
		pDP->SetCameraToolsInterface(pCDTI_MF);
		break;
	case CT_DETECTTOOLS_VFW:
		pDP->SetCameraToolsInterface(pCDTI_VFW);
		break;
	default:
		break;
	}
	pDP->start();
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

int CameraTools::IsWaterMaskSizeOk(const int& height, const int& width)
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

int CameraTools::GifToMat(std::vector<cv::Mat>& gifImgs, 
						  std::vector<cv::Mat>& maskImgs, 
						  std::string filename)
{
	FreeImage_Initialise(); 
	FREE_IMAGE_FORMAT fif = FIF_GIF;
	FIBITMAP* fiBmp1 = FreeImage_Load(fif, filename.c_str(), GIF_DEFAULT);
	FIMULTIBITMAP * fiBmp = FreeImage_OpenMultiBitmap(fif, filename.c_str(), 0, 1, 0, GIF_PLAYBACK);

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
	cv::Mat img;
	QImage Qimg;
	m_Dst->GetShowImageBuffer(img);

	if (m_Dst->GetImageStyle() == DP_IMAGESTYLE_GRAY) {
		Qimg = QImage(img.cols, img.rows, QImage::Format_Indexed8);
		Qimg = QImage(img.cols, img.rows, QImage::Format_Indexed8);
		Qimg.setColorCount(256);
		for (int i = 0; i < 256; i++)
		{
			Qimg.setColor(i, qRgb(i, i, i));
		}
		uchar *pSrc = img.data;
		for (int row = 0; row < img.rows; row++)
		{
			uchar *pDest = Qimg.scanLine(row);
			memcpy(pDest, pSrc, img.cols);
			pSrc += img.step;
		}
	}
	else {
		Qimg = QImage((uint8_t*)(img.data), img.cols,
			img.rows, img.cols*img.channels(), QImage::Format_RGB888);
	}
	ui.label_show_picture->resize(Qimg.size());
	QPixmap tempImg = QPixmap::fromImage(Qimg);
	ui.label_show_picture->setPixmap(tempImg);
}

void CameraTools::button_startshow_click()
{
///	VFWTools vt;
//	vt.ListCameraDevice();
	
	QString str = ui.pushButton_StartShow->text();
	if (str == QStringLiteral("��ʼԤ��")) {
		m_Dst = new DataProcess();
		connect(m_Dst, SIGNAL(SendUpdataImgMsg()), this,
			SLOT(paint_img()), Qt::QueuedConnection);
		connect(m_Dst, SIGNAL(SendAbort(int)), this,
			SLOT(get_abort(int)), Qt::QueuedConnection);
		StartPreview(m_Dst);
		WriteRecord(QStringLiteral("��ʼԤ��"));
		m_CaptureState = CT_STARTPREVIEW;
		UpdateControl(CT_STARTPREVIEW);
		ui.pushButton_StartShow->setText(QStringLiteral("ֹͣԤ��"));
	}
	else {
		str = ui.pushButton_StartCapture->text();
		if (str == QStringLiteral("ֹͣ¼��")) {
			QMessageBox::about(NULL, QStringLiteral("��ʾ"), 
				QStringLiteral("����ֹͣ¼��"));
			return ;
		}
		m_Dst->SetThreadStop(true);
		WriteRecord(QStringLiteral("����ֹͣԤ�������Ժ󡭡�"));
		m_CaptureState = CT_STOPPREVIEW;
		UpdateControl(CT_STOPPREVIEW);
		ui.pushButton_addmask->setText(QStringLiteral("���ˮӡ"));
		ui.pushButton_StartShow->setText(QStringLiteral("��ʼԤ��"));
	}
}

void CameraTools::button_startcapture_click()
{
	QString str = ui.pushButton_StartCapture->text();
	if (str == QStringLiteral("��ʼ¼��")) {
		m_IsPainting = CT_ISPAINTING_NO;
		QString file_path = QFileDialog::getExistingDirectory(this,
			QStringLiteral("��ѡ���ļ�����·��"), "./");
		m_IsPainting = CT_ISPAINTING_YES;
		if (file_path.isEmpty()) {
			return;
		}
		else {
			m_Dst->SetSaveVideoPath(file_path.toStdString());
			m_Dst->SetSaveVideoFlag(DP_SAVEVIDEO_INIT);
		}
		WriteRecord(QStringLiteral("��ʼ¼��"));
		WriteRecord(QStringLiteral("�洢λ�ã�")+file_path);
		ui.pushButton_StartCapture->setText(QStringLiteral("ֹͣ¼��"));
	}
	else {
		m_Dst->SetSaveVideoFlag(DP_SAVEVIDEO_END);
		WriteRecord(QStringLiteral("ֹͣ¼��"));
		ui.pushButton_StartCapture->setText(QStringLiteral("��ʼ¼��"));
	}
}


void CameraTools::button_grab_click()
{
	if (m_Dst->GetGrabImgFlag() == DP_GRAB_READY) {
		WriteRecord(QStringLiteral("ץ��"));
		m_Dst->SetGrabImgFlag(DP_GRAB_PROCESSING);
	}
}

void CameraTools::button_addmask_click()
{
	QString str = ui.pushButton_addmask->text();
	if (str == QStringLiteral("���ˮӡ")) {
		m_IsPainting = CT_ISPAINTING_NO;
		QString path = QFileDialog::getOpenFileName(this,
			QStringLiteral("ѡ����ӵ�ˮӡͼ��"),
			".",
			tr("Image Files(*.gif *.jpg *.png)"));
		m_IsPainting = CT_ISPAINTING_YES;
		if (path.length() == 0) {
			m_Dst->SetWaterMaskFlag(DP_WATERMASK_NONE);
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
				WriteRecord(QStringLiteral("ˮӡ�ߴ粻����"));
				return;
			}
			m_Dst->SetWaterMaskGifImg(tempGif);
			m_Dst->SetMaskGifImg(tempMask);
			m_Dst->SetWaterMaskFlag(DP_WATERMASK_GIF);
		}
		else {
			cv::Mat srcImg, maskImg;
			srcImg = cv::imread(filepath);
			if (IsWaterMaskSizeOk(srcImg.rows, srcImg.cols)
				== CT_WATERMASK_ERROR) {
				WriteRecord(QStringLiteral("ˮӡ�ߴ粻����"));
				return;
			}
			m_Dst->SetWaterMaskImg(srcImg);
			maskImg = cv::imread(filepath, 0);
			m_Dst->SetMaskImg(maskImg);
			m_Dst->SetWaterMaskFlag(DP_WATERMASK_RGB);
		}
		WriteRecord(QStringLiteral("���ˮӡ"));
		ui.horizontalSlider_transparency->setVisible(true);
		ui.pushButton_addmask->setText(QStringLiteral("ɾ��ˮӡ"));
	}
	else {
		m_Dst->SetWaterMaskFlag(DP_WATERMASK_NONE);
		WriteRecord(QStringLiteral("ɾ��ˮӡ"));
		ui.horizontalSlider_transparency->setVisible(false);
		ui.pushButton_addmask->setText(QStringLiteral("���ˮӡ"));
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
		m_Dst->SetImageStyle(DP_IMAGESTYLE_NORMAL);
		WriteRecord(QStringLiteral("ѡ���������"));
		break;
	case 1:
		m_Dst->SetImageStyle(DP_IMAGESTYLE_GRAY);
		WriteRecord(QStringLiteral("ѡ��ڰ׷��"));
		break;
	case 2:
		m_Dst->SetImageStyle(DP_IMAGESTYLE_OIL);
		WriteRecord(QStringLiteral("ѡ���ͻ����"));
		break;
	}
}

void CameraTools::combobox_detecttools_change()
{
	int index = ui.comboBox_DetectTools->currentIndex();
	switch (index)
	{
	case 0:
		m_DetectTools = CT_DETECTTOOLS_DS;
		break;
	case 1:
		m_DetectTools = CT_DETECTTOOLS_MF;
		break;
	case 2:
		m_DetectTools = CT_DETECTTOOLS_VFW;
		break;
	default:
		break;
	}
	DetectCamera();
}

void CameraTools::slider_value_change()
{
	m_Dst->SetTransparency(ui.horizontalSlider_transparency->value());
}

void CameraTools::get_abort(int ret)
{
	QString str;
	if (ret < 0) {
		WriteRecord(QStringLiteral("��ȡ����ͷ���ݴ���"));
		str = ui.pushButton_StartCapture->text();
		if (str == QStringLiteral("ֹͣ¼��")) {
			m_Dst->SetSaveVideoFlag(DP_SAVEVIDEO_END);
			WriteRecord(QStringLiteral("ֹͣ¼��"));
			ui.pushButton_StartCapture->setText(QStringLiteral("��ʼ¼��"));
		}
	}
	str = ui.pushButton_addmask->text();
	if (str == QStringLiteral("ɾ��ˮӡ")) {
		m_Dst->SetWaterMaskFlag(DP_WATERMASK_NONE);
		WriteRecord(QStringLiteral("ɾ��ˮӡ"));
		ui.horizontalSlider_transparency->setVisible(false);
		ui.pushButton_addmask->setText(QStringLiteral("���ˮӡ"));
	}
	UpdateControl(CT_STOPPREVIEW);
	ui.pushButton_StartShow->setText(QStringLiteral("��ʼԤ��"));
	WriteRecord(QStringLiteral("��ֹͣԤ��"));
//	delete m_Dst;
}

void CameraTools::checkbox_filter()
{
	if (ui.checkBox_Filter->isChecked()) {
		m_Dst->SetNoiseOptimizationFlag(DP_NOISEOPTIMIZATION_YES);
	}
	else {
		m_Dst->SetNoiseOptimizationFlag(DP_NOISEOPTIMIZATION_NO);
	}
}
