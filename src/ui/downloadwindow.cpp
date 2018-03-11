#include "downloadwindow.h"
#include "ui_downloadwindow.h"

DownloadWindow::DownloadWindow(QWidget* parent) : QWidget(parent), ui(new Ui::DownloadWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    
    on_comboMode_activated(ui->comboMode->currentText());
}



void DownloadWindow::on_comboMode_activated(const QString& text)
{
    if (text == "SocketCAN")
    {
        ui->labelSocketCAN->setVisible(true);
        ui->editSocketCAN->setVisible(true);
    }
    else
    {
        ui->labelSocketCAN->setVisible(false);
        ui->editSocketCAN->setVisible(false);
    }
}



DownloadWindow::~DownloadWindow()
{
    delete ui;
}
