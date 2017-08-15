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

	void SetDataProcessPtr(DataProcess* pDP);		// ��ȡ��ǰDataProcess��Ķ���ָ��
	void SetPaintingFlag(int flag);					// �����Ƿ��ͼ��ʾͼ��ı�־
	cv::Mat GetPaintImage();						// �õ���ǰӦ�û��Ƶ�ͼ��

protected:
	void initializeGL()  Q_DECL_OVERRIDE;			// OpenGL��ʼ������
	void resizeGL(int w, int h)  Q_DECL_OVERRIDE;	// ������������С�仯ʱ�Ĵ�����
	void paintGL()  Q_DECL_OVERRIDE;				// ���ƺ��������ڶ�ͼ����л���

private:
	DataProcess* m_pDP;
	int m_paintFlag;
};

#endif