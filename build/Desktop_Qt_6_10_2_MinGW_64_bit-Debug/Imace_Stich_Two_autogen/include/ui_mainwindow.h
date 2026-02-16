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
#include "maincampus.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout_2;
    maincampus *graphicsView;
    QGridLayout *GridLayout;
    QPushButton *pushButton_2;
    QLabel *label_1;
    QSlider *sliderOpacity2;
    QPushButton *pushButton_1;
    QPushButton *pushButton_Calc1;
    QSpinBox *spinOpacity2;
    QComboBox *comboBox;
    QSlider *sliderOpacity1;
    QSpinBox *spinOpacity1;
    QLabel *label_3;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_4;
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
        pushButton_2 = new QPushButton(centralwidget);
        pushButton_2->setObjectName("pushButton_2");

        GridLayout->addWidget(pushButton_2, 4, 0, 1, 3);

        label_1 = new QLabel(centralwidget);
        label_1->setObjectName("label_1");
        label_1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label_1, 2, 0, 1, 3);

        sliderOpacity2 = new QSlider(centralwidget);
        sliderOpacity2->setObjectName("sliderOpacity2");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(sliderOpacity2->sizePolicy().hasHeightForWidth());
        sliderOpacity2->setSizePolicy(sizePolicy1);
        sliderOpacity2->setOrientation(Qt::Orientation::Horizontal);

        GridLayout->addWidget(sliderOpacity2, 14, 0, 1, 1);

        pushButton_1 = new QPushButton(centralwidget);
        pushButton_1->setObjectName("pushButton_1");

        GridLayout->addWidget(pushButton_1, 3, 0, 1, 3);

        pushButton_Calc1 = new QPushButton(centralwidget);
        pushButton_Calc1->setObjectName("pushButton_Calc1");

        GridLayout->addWidget(pushButton_Calc1, 6, 0, 1, 3);

        spinOpacity2 = new QSpinBox(centralwidget);
        spinOpacity2->setObjectName("spinOpacity2");

        GridLayout->addWidget(spinOpacity2, 14, 2, 1, 1);

        comboBox = new QComboBox(centralwidget);
        comboBox->setObjectName("comboBox");

        GridLayout->addWidget(comboBox, 5, 2, 1, 1);

        sliderOpacity1 = new QSlider(centralwidget);
        sliderOpacity1->setObjectName("sliderOpacity1");
        sizePolicy1.setHeightForWidth(sliderOpacity1->sizePolicy().hasHeightForWidth());
        sliderOpacity1->setSizePolicy(sizePolicy1);
        sliderOpacity1->setOrientation(Qt::Orientation::Horizontal);

        GridLayout->addWidget(sliderOpacity1, 12, 0, 1, 1);

        spinOpacity1 = new QSpinBox(centralwidget);
        spinOpacity1->setObjectName("spinOpacity1");

        GridLayout->addWidget(spinOpacity1, 12, 2, 1, 1);

        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");
        label_3->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label_3, 13, 0, 1, 3);

        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label, 5, 0, 1, 1);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");
        label_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        GridLayout->addWidget(label_2, 11, 0, 1, 3);

        label_4 = new QLabel(centralwidget);
        label_4->setObjectName("label_4");

        GridLayout->addWidget(label_4, 7, 0, 1, 1);

        GridLayout->setRowStretch(7, 1);

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
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Image Sticher Two", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "Swap Back / Front", nullptr));
        label_1->setText(QCoreApplication::translate("MainWindow", "Control Box", nullptr));
        pushButton_1->setText(QCoreApplication::translate("MainWindow", "Open Image(s)", nullptr));
        pushButton_Calc1->setText(QCoreApplication::translate("MainWindow", "Calc. (\344\275\215\347\233\270\347\233\270\351\226\242)", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Image 2 Transparency", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Background Color", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Image 1 Transparency", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
