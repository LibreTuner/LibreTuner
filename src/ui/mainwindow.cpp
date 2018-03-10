#include "mainwindow.h"
#include "ui_mainwindow.h"


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



void MainWindow::on_actionCAN_Log_triggered()
{
    canViewer_.show();
}



void MainWindow::closeEvent(QCloseEvent* event)
{
    canViewer_.close();
}
