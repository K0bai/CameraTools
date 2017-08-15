#ifndef _OGLWIDGET_H
#define _OGLWIDGET_H

#include <QOpenGLWidget>
#include "DataProcess.h"

#define OGLW_PAINTING_NO 0
#define OGLW_PAINTING_YES 1

class OGLWidget :
	public QOpenGLWidget
{
public:
	OGLWidget(QWidget *parent = 0);
	~OGLWidget();

	void SetDataProcessPtr(DataProcess* pDP);		// 获取当前DataProcess类的对象指针
	void SetPaintingFlag(int flag);					// 设置是否绘图显示图像的标志
	cv::Mat GetPaintImage();						// 得到当前应该绘制的图像

protected:
	void initializeGL()  Q_DECL_OVERRIDE;			// OpenGL初始化函数
	void resizeGL(int w, int h)  Q_DECL_OVERRIDE;	// 当部件发生大小变化时的处理函数
	void paintGL()  Q_DECL_OVERRIDE;				// 绘制函数，用于对图像进行绘制

private:
	DataProcess* m_pDP;
	int m_paintFlag;
};

#endif