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

DownloadWindow::DownloadWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::DownloadWindow) {
  ui->setupUi(this);
  setWindowFlags(Qt::Window);

  layout()->setSizeConstraint(QLayout::SetFixedSize);

  // Populate vehicle combo
  ui->comboVehicle->clear();
  DefinitionPtr *defs = DefinitionManager::get()->definitions();
  for (int i = 0; i < DefinitionManager::get()->count(); ++i) {
    ui->comboVehicle->addItem(QString::fromStdString(defs[i]->name()),
                              QVariant::fromValue(defs[i]));
  }

  on_comboMode_activated(ui->comboMode->currentIndex());
}

void DownloadWindow::on_comboMode_activated(int index) {
  if (index == 0) {
    // SocketCAN
    ui->labelSocketCAN->setVisible(true);
    ui->editSocketCAN->setVisible(true);
  } else {
    ui->labelSocketCAN->setVisible(false);
    ui->editSocketCAN->setVisible(false);
  }
}

void DownloadWindow::start() {
  definition_ = ui->comboVehicle->currentData().value<DefinitionPtr>();

  name_ = ui->lineName->text().toStdString();
  switch (ui->comboMode->currentIndex()) {
#ifdef WITH_SOCKETCAN
  case 0: {
    // SocketCAN
    downloadInterface_ = DownloadInterface::createSocketCan(
        this, ui->editSocketCAN->text().toStdString(), definition_);
    if (!downloadInterface_) {
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
  default: {
    QMessageBox msgBox;
    msgBox.setText("The mode \"" + ui->comboMode->currentText() +
                   "\" is currently unsupported on this platform.");
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("Unsupported communication mode");
    // msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    break;
  }
  }
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

  ui->buttonBack->setVisible(true);
  ui->buttonContinue->setVisible(true);
  ui->stackedWidget->setCurrentIndex(1);
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
  hide();
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
  if (ui->stackedWidget->currentIndex() < ui->stackedWidget->count() - 2) {
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
    ui->buttonBack->setText(tr("Back"));
    return;
  }

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

void DownloadWindow::closeEvent(QCloseEvent *event) {
  ui->buttonBack->setVisible(true);
  ui->buttonContinue->setVisible(true);

  downloadInterface_.reset();
}

void DownloadWindow::showEvent(QShowEvent *event) {
  ui->stackedWidget->setCurrentIndex(0);
  ui->buttonBack->setText(tr("Cancel"));
}

DownloadWindow::~DownloadWindow() { delete ui; }
