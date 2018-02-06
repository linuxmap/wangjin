#include "mainwindow.h"
#include "ui_mainwindow.h"

int module_init();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/// init all components
int MainWindow::init()
{
    int ret = module_init();
    return ret;
}

