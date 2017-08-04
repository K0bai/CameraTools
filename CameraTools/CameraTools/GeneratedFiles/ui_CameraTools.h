/********************************************************************************
** Form generated from reading UI file 'CameraTools.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CAMERATOOLS_H
#define UI_CAMERATOOLS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CameraToolsClass
{
public:
    QWidget *centralWidget;
    QGroupBox *groupBox;
    QComboBox *comboBox_Camera;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QComboBox *comboBox_DataType;
    QComboBox *comboBox_DataResolution;
    QComboBox *comboBox_DataBit;
    QComboBox *comboBox_DataFps;
    QPushButton *pushButton_StartShow;
    QLabel *label_show_picture;
    QPushButton *pushButton_Grab;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *CameraToolsClass)
    {
        if (CameraToolsClass->objectName().isEmpty())
            CameraToolsClass->setObjectName(QStringLiteral("CameraToolsClass"));
        CameraToolsClass->resize(698, 649);
        centralWidget = new QWidget(CameraToolsClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 671, 111));
        comboBox_Camera = new QComboBox(groupBox);
        comboBox_Camera->setObjectName(QStringLiteral("comboBox_Camera"));
        comboBox_Camera->setGeometry(QRect(70, 20, 181, 22));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 20, 54, 12));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(20, 60, 61, 16));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(180, 60, 51, 16));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(350, 60, 61, 16));
        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(520, 60, 31, 16));
        comboBox_DataType = new QComboBox(groupBox);
        comboBox_DataType->setObjectName(QStringLiteral("comboBox_DataType"));
        comboBox_DataType->setGeometry(QRect(80, 60, 81, 22));
        comboBox_DataResolution = new QComboBox(groupBox);
        comboBox_DataResolution->setObjectName(QStringLiteral("comboBox_DataResolution"));
        comboBox_DataResolution->setGeometry(QRect(230, 60, 101, 22));
        comboBox_DataBit = new QComboBox(groupBox);
        comboBox_DataBit->setObjectName(QStringLiteral("comboBox_DataBit"));
        comboBox_DataBit->setGeometry(QRect(420, 60, 69, 22));
        comboBox_DataFps = new QComboBox(groupBox);
        comboBox_DataFps->setObjectName(QStringLiteral("comboBox_DataFps"));
        comboBox_DataFps->setGeometry(QRect(560, 60, 69, 22));
        pushButton_StartShow = new QPushButton(centralWidget);
        pushButton_StartShow->setObjectName(QStringLiteral("pushButton_StartShow"));
        pushButton_StartShow->setGeometry(QRect(10, 140, 75, 23));
        label_show_picture = new QLabel(centralWidget);
        label_show_picture->setObjectName(QStringLiteral("label_show_picture"));
        label_show_picture->setGeometry(QRect(10, 180, 671, 371));
        label_show_picture->setAutoFillBackground(false);
        label_show_picture->setFrameShape(QFrame::Box);
        label_show_picture->setFrameShadow(QFrame::Raised);
        label_show_picture->setAlignment(Qt::AlignCenter);
        pushButton_Grab = new QPushButton(centralWidget);
        pushButton_Grab->setObjectName(QStringLiteral("pushButton_Grab"));
        pushButton_Grab->setGeometry(QRect(100, 140, 75, 23));
        CameraToolsClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(CameraToolsClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 698, 23));
        CameraToolsClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(CameraToolsClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        CameraToolsClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(CameraToolsClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        CameraToolsClass->setStatusBar(statusBar);

        retranslateUi(CameraToolsClass);
        QObject::connect(pushButton_StartShow, SIGNAL(clicked()), CameraToolsClass, SLOT(button_startshow_click()));
        QObject::connect(pushButton_Grab, SIGNAL(clicked()), CameraToolsClass, SLOT(button_grab_click()));
        QObject::connect(comboBox_Camera, SIGNAL(currentIndexChanged(int)), CameraToolsClass, SLOT(combobox_camera_change()));

        QMetaObject::connectSlotsByName(CameraToolsClass);
    } // setupUi

    void retranslateUi(QMainWindow *CameraToolsClass)
    {
        CameraToolsClass->setWindowTitle(QApplication::translate("CameraToolsClass", "CameraTools", 0));
        groupBox->setTitle(QApplication::translate("CameraToolsClass", "\346\221\204\345\203\217\345\244\264\344\277\241\346\201\257", 0));
        label->setText(QApplication::translate("CameraToolsClass", "\346\221\204\345\203\217\345\244\264\357\274\232", 0));
        label_2->setText(QApplication::translate("CameraToolsClass", "\346\225\260\346\215\256\347\261\273\345\236\213\357\274\232", 0));
        label_3->setText(QApplication::translate("CameraToolsClass", "\345\210\206\350\276\250\347\216\207\357\274\232", 0));
        label_4->setText(QApplication::translate("CameraToolsClass", "\345\233\276\345\203\217\344\275\215\346\267\261\357\274\232", 0));
        label_5->setText(QApplication::translate("CameraToolsClass", "\345\270\247\347\216\207\357\274\232", 0));
        pushButton_StartShow->setText(QApplication::translate("CameraToolsClass", "\345\274\200\345\247\213\346\230\276\347\244\272", 0));
        label_show_picture->setText(QApplication::translate("CameraToolsClass", "\346\230\276\347\244\272\345\233\276\347\211\207", 0));
        pushButton_Grab->setText(QApplication::translate("CameraToolsClass", "\346\212\223\346\213\215", 0));
    } // retranslateUi

};

namespace Ui {
    class CameraToolsClass: public Ui_CameraToolsClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAMERATOOLS_H
