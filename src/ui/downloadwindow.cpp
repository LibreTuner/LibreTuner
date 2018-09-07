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
#include "ui_downloadwindow.h"

#include "definitions/definition.h"
#include "definitions/definitionmanager.h"
#include "logger.h"
#include "protocols/socketcaninterface.h"
#include "vehicle.h"

#include <QMessageBox>
#include <utility>

Q_DECLARE_METATYPE(DefinitionPtr)

DownloadWindow::DownloadWindow(std::unique_ptr<DownloadInterface> &&downloader,
                               const Vehicle &vehicle, QWidget *parent)
    : QDialog(parent), ui(new Ui::DownloadWindow),
      downloadInterface_(std::move(downloader)),
      definition_(vehicle.definition) {
    ui->setupUi(this);

    ui->labelVehicle->setText(QString::fromStdString(definition_->name()));
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
        RomManager::get()->addRom(ui->lineName->text().toStdString(),
                                  definition_, downloadInterface_->data());
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
}
