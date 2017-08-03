#include "CameraTools.h"
#include <qmessagebox.h>

CameraTools::CameraTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	b_grab = false;

	DirectShowTools ds;
	std::vector<CameraDeviceInfo> camera_device_list;
	camera_device_list = ds.ListCameraDevice();

	if (camera_device_list.size() != 0) {
		for (int i = 0; i < camera_device_list.size(); ++i)
		{
			ui.comboBox_Camera->addItems((QStringList)camera_device_list[i].friend_name.c_str());
		}
		ShowCameraInfo(camera_device_list, 1);
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

void CameraTools::paint_img(QImage src_img)
{
	ui.label_show_picture->resize(src_img.size());
	ui.label_show_picture->setPixmap(QPixmap::fromImage(src_img));
	if (b_grab) {
		b_grab = false;
		src_img.save("2.jpeg");
	}
}

void CameraTools::button_startshow_click()
{
	QMessageBox::about(NULL, "1", "test");

	ds1 = new DirectShowTools();
//	ds1->ConfigCamera();
	connect(ds1, SIGNAL(send_image_data(QImage)), this,
		SLOT(paint_img(QImage)), Qt::QueuedConnection);
	ds1->start();
}

void CameraTools::button_grab_click()
{
	b_grab = true;
}