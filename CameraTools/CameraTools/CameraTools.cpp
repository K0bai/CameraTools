#include "CameraTools.h"
#include <qmessagebox.h>
#include <qdatetime.h>

CameraTools::CameraTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	b_grab = false;

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
	cv::Mat src_img = cv::Mat::zeros(480, 640, CV_8UC3);

	cv::Mat src1 = cv::imread("logo.png");
	ds1->m_WaterMaskImg = cv::Mat::zeros(480, 640, CV_8UC3);
	src1.copyTo(ds1->m_WaterMaskImg(cv::Rect(10, 10, src1.cols, src1.rows)));
	ds1->m_MaskFlag = true;
}

void CameraTools::combobox_camera_change()
{
	int i = ui.comboBox_Camera->currentIndex();
	ShowCameraInfo(camera_device_list, i);
}