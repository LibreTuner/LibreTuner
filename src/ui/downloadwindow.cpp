#include "downloadwindow.h"
#include "ui_downloadwindow.h"

#include <QMessageBox>
#include "protocols/socketcaninterface.h"
#include "protocols/isotpinterface.h"



DownloadWindow::DownloadWindow(QWidget* parent) : QWidget(parent), ui(new Ui::DownloadWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    
    on_comboMode_activated(ui->comboMode->currentIndex());
}



void DownloadWindow::on_comboMode_activated(int index)
{
    if (index == 0)
    {
        // SocketCAN
        ui->labelSocketCAN->setVisible(true);
        ui->editSocketCAN->setVisible(true);
    }
    else
    {
        ui->labelSocketCAN->setVisible(false);
        ui->editSocketCAN->setVisible(false);
    }
}



void DownloadWindow::start()
{
    switch(ui->comboVehicle->currentIndex())
    {
        case 0: // Mazdaspeed 6
            vehicle_ = ROM_MAZDASPEED6;
            break;
    }
    name_ = ui->lineName->text().toStdString();
    switch (ui->comboMode->currentIndex())
    {
#ifdef WITH_SOCKETCAN
        case 0:
        {
            // SocketCAN
            downloadInterface_ = DownloadInterface::createSocketCan(this, ui->editSocketCAN->text().toStdString(), vehicle_);
            if (!downloadInterface_)
            {
                // The interface should have called the downloadError callback
                return;
            }
            
            ui->buttonBack->setVisible(false);
            ui->buttonContinue->setVisible(false);

            downloadInterface_->download();
            
            ui->stackedWidget->setCurrentIndex(2);
            
            break;
        }
#endif
#ifdef WITH_J2534
        case 1:
            // J2534
            break;
#endif
        default:
        {
            QMessageBox msgBox;
            msgBox.setText("The mode \"" + ui->comboMode->currentText() + "\" is currently unsupported on this platform.");
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowTitle("Unsupported communication mode");
            // msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            break;
        }
    }
}



void DownloadWindow::mainDownloadError(const QString& error)
{
    // runs in main thread
    QMessageBox msgBox;
    msgBox.setText("Error while downloading firmware: " + error);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("Download error");
    // msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    
    downloadInterface_.reset();
    
    ui->buttonBack->setVisible(true);
    ui->buttonContinue->setVisible(true);
    ui->stackedWidget->setCurrentIndex(1);
}



void DownloadWindow::downloadError(const QString& error)
{
    QMetaObject::invokeMethod(this, "mainDownloadError", Qt::QueuedConnection, Q_ARG(QString ,error));
    downloadInterface_.reset();
}



void DownloadWindow::mainOnCompletion()
{
    QMessageBox msgBox;
    msgBox.setText("Download complete");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("Download complete");
    msgBox.exec();
    
    hide();
}



void DownloadWindow::onCompletion(const uint8_t* data, size_t length)
{
    if (!RomManager::get()->addRom(name_, vehicle_, data, length))
    {
        QMessageBox msgBox;
        msgBox.setText("Failed to save rom: " + RomManager::get()->lastError());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Save Error");
        downloadInterface_.reset();
        QMetaObject::invokeMethod(&msgBox, "exec", Qt::QueuedConnection);
        QMetaObject::invokeMethod(this, "hide", Qt::QueuedConnection);
    }
    else
    {
        downloadInterface_.reset();
        QMetaObject::invokeMethod(this, "mainOnCompletion", Qt::QueuedConnection);
    }
}



void DownloadWindow::updateProgress(float progress)
{
    QMetaObject::invokeMethod(ui->progressDownload, "setValue", Qt::QueuedConnection, Q_ARG(int, (int)progress));
}



void DownloadWindow::on_buttonContinue_clicked()
{
    if (ui->stackedWidget->currentIndex() < ui->stackedWidget->count() - 2)
    {
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
        ui->buttonBack->setText(tr("Back"));
        return;
    }
    
    // Last page
    start();
}



void DownloadWindow::on_buttonBack_clicked()
{
    if (ui->stackedWidget->currentIndex() == 0)
    {
        close();
        return;
    }
    
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() - 1);
    if (ui->stackedWidget->currentIndex() == 0)
    {
        ui->buttonBack->setText(tr("Cancel"));
    }
}



void DownloadWindow::closeEvent(QCloseEvent* event)
{
    ui->buttonBack->setVisible(true);
    ui->buttonContinue->setVisible(true);
    
    downloadInterface_.reset();
}



void DownloadWindow::showEvent(QShowEvent* event)
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->buttonBack->setText(tr("Cancel"));
}



DownloadWindow::~DownloadWindow()
{
    delete ui;
}
