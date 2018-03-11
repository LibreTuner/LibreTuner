#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "romwidget.h"
#include "flowlayout.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), downloadWindow_(new DownloadWindow(this))
{
    ui->setupUi(this);
    
    //ui->listRoms->setModel(&romModel_);
    
    for (int i = 1; i < 10; ++i)
    {
        ui->romLayout->addWidget(new RomWidget(ui->tabRoms));
    }
}



MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_actionCAN_Log_triggered()
{
    canViewer_.show();
}



void MainWindow::on_buttonDownloadRom_clicked()
{
    downloadWindow_->show();
}



void MainWindow::closeEvent(QCloseEvent* event)
{
    canViewer_.close();
}
