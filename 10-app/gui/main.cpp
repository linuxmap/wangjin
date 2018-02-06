#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    int ret = w.init();
    if (ret < 0)
    {
        qDebug()<<"init mainwindow failed,ret:"<<ret<<"\n";
    }
    w.show();
    qDebug()<<"GUI start successfull\n";

    return a.exec();
}
