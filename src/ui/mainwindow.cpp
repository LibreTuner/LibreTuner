#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "romwidget.h"
#include "flowlayout.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), downloadWindow_(new DownloadWindow(this))
{
    ui->setupUi(this);
    
    connect(RomManager::get(), &RomManager::updateRoms, this, &MainWindow::updateRoms);
    updateRoms();
}



void MainWindow::updateRoms()
{
    QLayoutItem *child;
    while ((child = ui->romLayout->takeAt(0)) != 0)
    {
        delete child;
    }
    
    for (RomDataPtr rom : RomManager::get()->roms())
    {
        ui->romLayout->addWidget(new RomWidget(rom));
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
