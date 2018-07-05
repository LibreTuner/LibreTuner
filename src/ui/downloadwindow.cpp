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
#include "protocols/socketcaninterface.h"

#include <QMessageBox>

Q_DECLARE_METATYPE(DefinitionPtr)

DownloadWindow::DownloadWindow(const DataLinkPtr &datalink, QWidget *parent)
    : QWidget(parent), datalink_(datalink), ui(new Ui::DownloadWindow) {
  ui->setupUi(this);
  setWindowFlags(Qt::Window);

  layout()->setSizeConstraint(QLayout::SetFixedSize);

  query();
}

void DownloadWindow::queryError(DataLink::Error error) {
  QMessageBox msg;
  msg.setText("Could not query vehicle type: " + QString::fromStdString(DataLink::strError(error)));
  msg.setWindowTitle("Query Error");
  msg.setIcon(QMessageBox::Critical);
  msg.exec();
  close();
}

void DownloadWindow::vehicleQueried(VehiclePtr vehicle) {
  ui->labelVehicle->setText("Found vehicle: " + QString::fromStdString(vehicle->name()));
  ui->vinLineEdit->setText(QString::fromStdString(vehicle->vin()));
  if (!vehicle->definition()) {
    QMessageBox msg;
    msg.setText("Unknown vehicle");
    msg.setWindowTitle("Unknown vehicle");
    msg.setIcon(QMessageBox::Critical);
    msg.exec();
    close();
    return;
  }
  definition_ = vehicle->definition();
  ui->buttonContinue->setEnabled(true);
}

void DownloadWindow::start() {
  name_ = ui->lineName->text().toStdString();
  // SocketCAN
  downloadInterface_ = DownloadInterface::create(
      this, datalink_, definition_);
  if (!downloadInterface_) {
    // The interface should have called the downloadError callback
    return;
  }

  ui->buttonBack->setEnabled(false);
  ui->buttonContinue->setEnabled(false);

  downloadInterface_->download();

  ui->stackedWidget->setCurrentIndex(1);
}

void DownloadWindow::mainDownloadError(const QString &error) {
  // runs in main thread
  QMessageBox msgBox;
  msgBox.setText("Error while downloading firmware: " + error);
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.setWindowTitle("Download error");
  // msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.exec();

  downloadInterface_.reset();

  ui->buttonBack->setEnabled(true);
  ui->buttonContinue->setEnabled(true);
  ui->stackedWidget->setCurrentIndex(0);
}

void DownloadWindow::downloadError(const QString &error) {
  QMetaObject::invokeMethod(this, "mainDownloadError", Qt::QueuedConnection,
                            Q_ARG(QString, error));
  downloadInterface_.reset();
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

void DownloadWindow::onCompletion(gsl::span<const uint8_t> data) {
  if (!RomManager::get()->addRom(name_, definition_, data)) {
    QMetaObject::invokeMethod(this, "mainOnCompletion", Qt::QueuedConnection,
                              Q_ARG(bool, false),
                              Q_ARG(QString, RomManager::get()->lastError()));
  } else {
    QMetaObject::invokeMethod(this, "mainOnCompletion", Qt::QueuedConnection,
                              Q_ARG(bool, true), Q_ARG(QString, QString()));
  }
  downloadInterface_.reset();
}

void DownloadWindow::updateProgress(float progress) {
  QMetaObject::invokeMethod(ui->progressDownload, "setValue",
                            Qt::QueuedConnection,
                            Q_ARG(int, (int)(progress * 100)));
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

Q_DECLARE_METATYPE(DataLink::Error)
Q_DECLARE_METATYPE(VehiclePtr)

void DownloadWindow::query() {
  ui->labelVehicle->setText("Querying vehicle... Please Wait");
  ui->buttonContinue->setEnabled(false);
  // Query vehicle
  qRegisterMetaType<DataLink::Error>("DataLink::Error");
  qRegisterMetaType<VehiclePtr>("VehiclePtr");
  datalink_->queryVehicle([this](DataLink::Error error, VehiclePtr vehicle) {
    if (error != DataLink::Error::Success) {
      QMetaObject::invokeMethod(this, "queryError", Qt::QueuedConnection, Q_ARG(DataLink::Error, error));
      return;
    }
    QMetaObject::invokeMethod(this, "vehicleQueried", Qt::QueuedConnection, Q_ARG(VehiclePtr, vehicle));
  });
}

DownloadWindow::~DownloadWindow() { delete ui; }
