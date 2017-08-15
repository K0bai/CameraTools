#include "OGLWidget.h"
#include "CameraTools.h"


OGLWidget::OGLWidget(QWidget * parent) : QOpenGLWidget(parent)
{
	m_paintFlag = OGLW_PAINTING_NO;
}


OGLWidget::~OGLWidget()
{
}

void OGLWidget::SetDataProcessPtr(DataProcess* pDP)
{
	m_pDP = pDP;
	m_paintFlag = OGLW_PAINTING_YES;
}

void OGLWidget::SetPaintingFlag(int flag)
{
	m_paintFlag = flag;
}

/*
 * 从缓冲队列中得到图像数据
 * 返回值：缓冲队列头部的图像数据
 */
cv::Mat OGLWidget::GetPaintImage()
{
	cv::Mat img;
	m_pDP->GetShowImageBuffer(img);
	return img.clone();
}

/*
 *
 */
void OGLWidget::initializeGL()
{
	glClearColor(255, 255, 255, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
}

/*
 *
 */
void OGLWidget::paintGL()
{
	if (m_paintFlag == OGLW_PAINTING_NO) {
		return;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	cv::Mat showImg = GetPaintImage();
	flip(showImg, showImg, 0);

	if (!showImg.empty()) {
		if (showImg.channels() == 1) {
			cv::cvtColor(showImg, showImg, cv::COLOR_GRAY2RGB);
			glDrawPixels(showImg.cols, showImg.rows, GL_RGB, GL_UNSIGNED_BYTE, showImg.data);
		}
		else {
			glDrawPixels(showImg.cols, showImg.rows, GL_RGB, GL_UNSIGNED_BYTE, showImg.data);
		}
	}
}

/*
 *
 */
void OGLWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	update();
}