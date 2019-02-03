/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "downloadwindow.h"


#include "definitions/definition.h"
#include "definitions/definitionmanager.h"
#include "logger.h"
#include "protocols/socketcaninterface.h"
#include "vehicle.h"
#include "libretuner.h"
#include "rommanager.h"
#include "download/downloader.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QThread>
#include <QProgressDialog>
#include <QComboBox>
#include <QFileDialog>

#include <utility>
#include <thread>
#include <atomic>


DownloadWindow::DownloadWindow(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("LibreTuner - Download"));
    auto *layout = new QVBoxLayout;

    comboPlatform_ = new QComboBox;
    comboPlatform_->setModel(DefinitionManager::get());
    
    auto *groupDetails = new QGroupBox(tr("ROM Details"));
    auto *form = new QFormLayout;
    //form->setContentsMargins(0, 4, 0, 4);
    
    lineName_ = new QLineEdit;
    lineId_ = new QLineEdit;

    form->addRow(tr("Platform"), comboPlatform_);
    form->addRow(tr("Name"), lineName_);
    form->addRow(tr("Id"), lineId_);
    
    groupDetails->setLayout(form);

    layout->addWidget(groupDetails);
    
    auto *buttonDownload = new QPushButton(tr("Download"));
    layout->addWidget(buttonDownload);
    
    connect(buttonDownload, &QPushButton::clicked, [this]() {
        download();
    });
    
    setLayout(layout);
}


void DownloadWindow::download()
{
    try {
        std::unique_ptr<PlatformLink> pLink = get_platform_link();
        if (!pLink) {
            throw std::runtime_error("Invalid platform or data link");
        }

        QProgressDialog progress(tr("Downloading ROM..."), tr("Abort"), 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setWindowTitle(tr("LibreTuner - Download"));
        progress.setValue(0);
        progress.show();


        std::unique_ptr<download::Downloader> downloader = pLink->downloader();
        downloader->setProgressCallback([&](float prog) {
            QMetaObject::invokeMethod(&progress, "setValue", Qt::QueuedConnection, Q_ARG(int, prog * 100));
        });

        bool success = false;
        std::atomic<bool> stopped(false);

        std::promise<void> p;
        std::future<void> future = p.get_future();

        std::thread worker([&]() {
            try {
                success = downloader->download();
            } catch (...) {
                success = false;
                p.set_exception(std::current_exception());
            }
            stopped = true;
        });

        bool canceled = false;

        while (!stopped) {
            QApplication::processEvents(QEventLoop::WaitForMoreEvents);
            if (progress.wasCanceled()) {
                downloader->cancel();
                canceled = true;
            }
        }
        worker.join();

        if (!success && !canceled) {
            future.get();
            throw std::runtime_error("Unknown error");
        } else {
            auto data = downloader->data();
            try {
                RomStore::get()->addRom(lineName_->text().toStdString(),
                                        pLink->definition(), data.first, data.second);
                QMessageBox(QMessageBox::Information, "Download Finished", "ROM downloaded successfully").exec();
            } catch (const std::runtime_error &err) {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Download Error");
                msgBox.setText("The ROM was downloaded, but an error occurred while saving. Would you like to save the binary data?");
                msgBox.setInformativeText(err.what());
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);

                int ret = msgBox.exec();
                if (ret == QMessageBox::Yes) {
                    QString fileName = QFileDialog::getSaveFileName(this,
                                                                    tr("Save ROM"), "",
                                                                    tr("Binary (*.bin);;All Files (*)"));
                    if (fileName.isEmpty()) {
                        return;
                    }

                    QFile file(fileName);
                    if (!file.open(QIODevice::WriteOnly)) {
                        QMessageBox::information(this, tr("Unable to open file"),
                                                 file.errorString());
                        return;
                    }
                    file.write(reinterpret_cast<const char*>(data.first), static_cast<qint64>(data.second));
                    file.close();
                }
            }
        }

        
    } catch (const std::runtime_error &err) {
        QMessageBox(QMessageBox::Critical, "Download Error", err.what()).exec();
    }
}



void DownloadWindow::closeEvent(QCloseEvent* event)
{
}


Q_DECLARE_METATYPE(definition::MainPtr)


std::unique_ptr<PlatformLink> DownloadWindow::get_platform_link() {
    datalink::Link *link = LT()->datalink();
    if (link == nullptr) {
        return nullptr;
    }

    QVariant var = comboPlatform_->currentData(Qt::UserRole);
    if (!var.canConvert<definition::MainPtr>()) {
        return nullptr;
    }

    auto platform = var.value<definition::MainPtr>();
    if (!platform) {
        return nullptr;
    }

    return std::make_unique<PlatformLink>(platform, *link);
}



DownloadWindow::~DownloadWindow()
= default;


/*
DownloadWindow::DownloadWindow(std::unique_ptr<DownloadInterface> &&downloader,
                               const Vehicle &vehicle, QWidget *parent)
    : QDialog(parent), ui(new Ui::DownloadWindow),
      downloadInterface_(std::move(downloader)),
      definition_(vehicle.definition) {
    ui->setupUi(this);

    ui->labelVehicle->setText(QString::fromStdString(definition_->name));
    ui->vinLineEdit->setText(QString::fromStdString(vehicle.vin));
}

void DownloadWindow::start() {
    if (worker_.joinable() || !downloadInterface_) {
        // Nope
        return;
    }
    downloadInterface_->setProgressCallback(
        [this](float progress) { updateProgress(progress); });

    ui->buttonBack->setEnabled(false);
    ui->buttonContinue->setEnabled(false);

    ui->stackedWidget->setCurrentIndex(1);
    ui->progressDownload->setValue(0);
    ui->labelETR->setText("");
    lastUpdate_ = std::chrono::steady_clock::now();
    worker_ = std::thread([this]() {
        try {
            if (downloadInterface_->download()) {
                onCompletion();
            }
        } catch (const std::exception &e) {
            downloadError(QString(e.what()));
        }
    });
}

void DownloadWindow::stop() {
    if (worker_.joinable()) {
        // Notify cancel
        Logger::info("Canceling download");
        downloadInterface_->cancel();
        worker_.join();
    }
    downloadInterface_.reset();
}

void DownloadWindow::closeEvent(QCloseEvent *event) { stop(); }

void DownloadWindow::mainDownloadError(const QString &error) {
    // runs in main thread
    QMessageBox msgBox;
    msgBox.setText("Error while downloading firmware: " + error);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("Download error");
    // msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();

    ui->buttonBack->setEnabled(true);
    ui->buttonContinue->setEnabled(true);
    ui->stackedWidget->setCurrentIndex(0);
}

void DownloadWindow::downloadError(const QString &error) {
    QMetaObject::invokeMethod(this, "mainDownloadError", Qt::QueuedConnection,
                              Q_ARG(QString, error));
}

void DownloadWindow::mainOnCompletion(bool success, const QString &error) {
    if (!success) {
        QMessageBox msgBox;
        msgBox.setText("Failed to save rom: " + error);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Save Error");
        msgBox.exec();
    } else {
        QMessageBox msgBox;
        msgBox.setText("Download complete");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle("Download complete");
        msgBox.exec();
    }
    close();
}

void DownloadWindow::onCompletion() {
    try {
        auto data = downloadInterface_->data();
        RomStore::get()->addRom(ui->lineName->text().toStdString(),
                                  definition_, data.first, data.second);
        QMetaObject::invokeMethod(this, "mainOnCompletion",
                                  Qt::QueuedConnection, Q_ARG(bool, true),
                                  Q_ARG(QString, QString()));
    } catch (const std::exception &e) {
        QMetaObject::invokeMethod(this, "mainOnCompletion",
                                  Qt::QueuedConnection, Q_ARG(bool, false),
                                  Q_ARG(QString, e.what()));
    }
}

void DownloadWindow::updateProgress(float progress) {
    QMetaObject::invokeMethod(ui->progressDownload, "setValue",
                              Qt::QueuedConnection,
                              Q_ARG(int, static_cast<int>(progress * 100)));

    std::chrono::steady_clock::duration dur =
        std::chrono::steady_clock::now() - lastUpdate_;
    float elapsed =
        (static_cast<float>(
             std::chrono::duration_cast<std::chrono::seconds>(dur).count()) /
         60.0f);
    float minutesRemaining = (elapsed * (1.0f - progress)) / progress;
    QMetaObject::invokeMethod(
        ui->labelETR, "setText", Qt::QueuedConnection,
        Q_ARG(QString, QString("Estimated time remaining: ") +
                           QString::number(minutesRemaining) +
                           QString(" minutes")));
}

void DownloadWindow::on_buttonContinue_clicked() {
    // Last page
    start();
}

void DownloadWindow::on_buttonBack_clicked() {
    if (ui->stackedWidget->currentIndex() == 0) {
        close();
        return;
    }

    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() - 1);
    if (ui->stackedWidget->currentIndex() == 0) {
        ui->buttonBack->setText(tr("Cancel"));
    }
}

DownloadWindow::~DownloadWindow() {
    stop();
    delete ui;
}*/
