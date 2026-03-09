/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "droparea.h"
#include "maincampus.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout_2;
    maincampus *graphicsView;
    QGridLayout *GridLayout;
    QPushButton *pushButton_4;
    QLabel *label_3;
    QLabel *label_2;
    QSlider *sliderOpacity1;
    QLabel *label_5;
    QLabel *label_9;
    QLabel *label_4;
    QLabel *label_8;
    QPushButton *pushButton_Calc1;
    QSlider *sliderOpacity2;
    QSpinBox *spinOpacity1;
    QPushButton *pushButton_2;
    QPushButton *pushButton_Calc2;
    QLabel *label_7;
    QPushButton *pushButton_3;
    QSpinBox *spinBoxSSIM;
    DropArea *dropArea;
    QLabel *label;
    QPushButton *pushButton_1;
    QComboBox *comboBox;
    QLabel *label_6;
    QSpinBox *spinOpacity2;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1000, 600);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        gridLayout_2 = new QGridLayout(centralwidget);
        gridLayout_2->setObjectName("gridLayout_2");
        graphicsView = new maincampus(centralwidget);
        graphicsView->setObjectName("graphicsView");

        gridLayout_2->addWidget(graphicsView, 0, 2, 1, 1);

        GridLayout = new QGridLayout();
        GridLayout->setObjectName("GridLayout");
        pushButton_4 = new QPushButton(centralwidget);
        pushButton_4->setObjectName("pushButton_4");

        GridLayout->addWidget(pushButton_4, 12, 0, 1, 2);

        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");
        label_3->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label_3, 16, 0, 1, 2);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");
        label_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label_2, 14, 0, 1, 2);

        sliderOpacity1 = new QSlider(centralwidget);
        sliderOpacity1->setObjectName("sliderOpacity1");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(sliderOpacity1->sizePolicy().hasHeightForWidth());
        sliderOpacity1->setSizePolicy(sizePolicy1);
        sliderOpacity1->setOrientation(Qt::Orientation::Horizontal);

        GridLayout->addWidget(sliderOpacity1, 15, 0, 1, 1);

        label_5 = new QLabel(centralwidget);
        label_5->setObjectName("label_5");

        GridLayout->addWidget(label_5, 7, 1, 1, 1);

        label_9 = new QLabel(centralwidget);
        label_9->setObjectName("label_9");
        label_9->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label_9, 9, 0, 1, 1);

        label_4 = new QLabel(centralwidget);
        label_4->setObjectName("label_4");
        label_4->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label_4, 7, 0, 1, 1);

        label_8 = new QLabel(centralwidget);
        label_8->setObjectName("label_8");

        GridLayout->addWidget(label_8, 13, 0, 1, 2);

        pushButton_Calc1 = new QPushButton(centralwidget);
        pushButton_Calc1->setObjectName("pushButton_Calc1");

        GridLayout->addWidget(pushButton_Calc1, 6, 0, 1, 2);

        sliderOpacity2 = new QSlider(centralwidget);
        sliderOpacity2->setObjectName("sliderOpacity2");
        sizePolicy1.setHeightForWidth(sliderOpacity2->sizePolicy().hasHeightForWidth());
        sliderOpacity2->setSizePolicy(sizePolicy1);
        sliderOpacity2->setOrientation(Qt::Orientation::Horizontal);

        GridLayout->addWidget(sliderOpacity2, 17, 0, 1, 1);

        spinOpacity1 = new QSpinBox(centralwidget);
        spinOpacity1->setObjectName("spinOpacity1");

        GridLayout->addWidget(spinOpacity1, 15, 1, 1, 1);

        pushButton_2 = new QPushButton(centralwidget);
        pushButton_2->setObjectName("pushButton_2");

        GridLayout->addWidget(pushButton_2, 4, 0, 1, 2);

        pushButton_Calc2 = new QPushButton(centralwidget);
        pushButton_Calc2->setObjectName("pushButton_Calc2");

        GridLayout->addWidget(pushButton_Calc2, 8, 0, 1, 2);

        label_7 = new QLabel(centralwidget);
        label_7->setObjectName("label_7");

        GridLayout->addWidget(label_7, 10, 1, 1, 1);

        pushButton_3 = new QPushButton(centralwidget);
        pushButton_3->setObjectName("pushButton_3");

        GridLayout->addWidget(pushButton_3, 11, 0, 1, 2);

        spinBoxSSIM = new QSpinBox(centralwidget);
        spinBoxSSIM->setObjectName("spinBoxSSIM");
        spinBoxSSIM->setMaximum(99999);
        spinBoxSSIM->setValue(10);

        GridLayout->addWidget(spinBoxSSIM, 9, 1, 1, 1);

        dropArea = new DropArea(centralwidget);
        dropArea->setObjectName("dropArea");

        GridLayout->addWidget(dropArea, 3, 0, 1, 2);

        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label, 5, 0, 1, 1);

        pushButton_1 = new QPushButton(centralwidget);
        pushButton_1->setObjectName("pushButton_1");

        GridLayout->addWidget(pushButton_1, 2, 0, 1, 2);

        comboBox = new QComboBox(centralwidget);
        comboBox->setObjectName("comboBox");

        GridLayout->addWidget(comboBox, 5, 1, 1, 1);

        label_6 = new QLabel(centralwidget);
        label_6->setObjectName("label_6");
        label_6->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label_6, 10, 0, 1, 1);

        spinOpacity2 = new QSpinBox(centralwidget);
        spinOpacity2->setObjectName("spinOpacity2");

        GridLayout->addWidget(spinOpacity2, 17, 1, 1, 1);

        GridLayout->setRowStretch(13, 1);

        gridLayout_2->addLayout(GridLayout, 0, 1, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1000, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Image Stitcher Two", nullptr));
        pushButton_4->setText(QCoreApplication::translate("MainWindow", "PNG export", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Image 2 Transparency", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Image 1 Transparency", nullptr));
        label_5->setText(QString());
        label_9->setText(QCoreApplication::translate("MainWindow", "\346\216\242\347\264\242\347\257\204\345\233\262 (pix)", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Stitch\345\223\201\350\263\252 (0-1)", nullptr));
        label_8->setText(QString());
        pushButton_Calc1->setText(QCoreApplication::translate("MainWindow", "Calc. Position (\344\275\215\347\233\270\347\233\270\351\226\242\346\263\225)", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "Swap Back / Front", nullptr));
        pushButton_Calc2->setText(QCoreApplication::translate("MainWindow", "Calc. Position (SSIM)", nullptr));
        label_7->setText(QString());
        pushButton_3->setText(QCoreApplication::translate("MainWindow", "\347\265\220\345\220\210", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Background Color", nullptr));
        pushButton_1->setText(QCoreApplication::translate("MainWindow", "Open Image(s)", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "SSIM", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
