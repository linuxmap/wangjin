/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QTreeView *treeView_playlist;
    QLabel *label_playlist;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLabel *label_video0;
    QLabel *label_video2;
    QLabel *label_video3;
    QLabel *label_video1;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1280, 720);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        treeView_playlist = new QTreeView(centralWidget);
        treeView_playlist->setObjectName(QStringLiteral("treeView_playlist"));
        treeView_playlist->setGeometry(QRect(0, 40, 191, 611));
        label_playlist = new QLabel(centralWidget);
        label_playlist->setObjectName(QStringLiteral("label_playlist"));
        label_playlist->setGeometry(QRect(0, 10, 71, 31));
        label_playlist->setTextFormat(Qt::PlainText);
        label_playlist->setAlignment(Qt::AlignCenter);
        gridLayoutWidget = new QWidget(centralWidget);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(190, 40, 1081, 611));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label_video0 = new QLabel(gridLayoutWidget);
        label_video0->setObjectName(QStringLiteral("label_video0"));
        label_video0->setFrameShape(QFrame::WinPanel);
        label_video0->setLineWidth(1);
        label_video0->setScaledContents(true);
        label_video0->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_video0, 0, 0, 1, 1);

        label_video2 = new QLabel(gridLayoutWidget);
        label_video2->setObjectName(QStringLiteral("label_video2"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_video2->sizePolicy().hasHeightForWidth());
        label_video2->setSizePolicy(sizePolicy);
        label_video2->setFrameShape(QFrame::WinPanel);

        gridLayout->addWidget(label_video2, 1, 0, 1, 1);

        label_video3 = new QLabel(gridLayoutWidget);
        label_video3->setObjectName(QStringLiteral("label_video3"));
        sizePolicy.setHeightForWidth(label_video3->sizePolicy().hasHeightForWidth());
        label_video3->setSizePolicy(sizePolicy);
        label_video3->setFrameShape(QFrame::WinPanel);
        label_video3->setLineWidth(1);
        label_video3->setMidLineWidth(0);

        gridLayout->addWidget(label_video3, 1, 1, 1, 1);

        label_video1 = new QLabel(gridLayoutWidget);
        label_video1->setObjectName(QStringLiteral("label_video1"));
        label_video1->setFrameShape(QFrame::WinPanel);

        gridLayout->addWidget(label_video1, 0, 1, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1280, 22));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "NETMARCH MEDIA CENTER", Q_NULLPTR));
        label_playlist->setText(QApplication::translate("MainWindow", "device list", Q_NULLPTR));
        label_video0->setText(QString());
        label_video2->setText(QString());
        label_video3->setText(QString());
        label_video1->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
