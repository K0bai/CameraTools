#include "CameraTools.h"
#include <qmessagebox.h>

CameraTools::CameraTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	DirectShowTools ds;
	std::vector<CameraDeviceInfo> camera_device_list;
	camera_device_list = ds.ListCameraDevice();

	if (camera_device_list.size() != 0) {
		for (int i = 0; i < camera_device_list.size(); ++i)
		{
			ui.comboBox_Camera->addItems((QStringList)camera_device_list[i].friend_name.c_str());
		}
		ShowCameraInfo(camera_device_list, 0);
	}
	
	
}
void CameraTools::ShowCameraInfo(std::vector<CameraDeviceInfo>& camera_info, int index)
{
	ui.comboBox_Camera->setCurrentIndex(index);

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

void CameraTools::button_startshow_click()
{
	QMessageBox::about(NULL, "1", "test");
	cv::Mat image;
	image = cv::imread("1.jpg", -1);
	cv::cvtColor(image, image, CV_BGR2RGB);
	QImage img = QImage((const unsigned char*)(image.data), image.cols, image.rows, image.cols*image.channels(), QImage::Format_RGB888);
//	ui.label_show_picture->clear();
	ui.label_show_picture->setPixmap(QPixmap::fromImage(img));
//	ui.label_show_picture->resize(ui.label_show_picture->pixmap()->size());
//	DirectShowTools *ds = new DirectShowTools();
//	ds->start();

}