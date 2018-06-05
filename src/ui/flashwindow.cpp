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

#include "flashwindow.h"
#include "ui_flashwindow.h"

#include "definitions/definition.h"
#include "flashable.h"
#include "flasher.h"

#ifdef WITH_SOCKETCAN
#include "protocols/socketcaninterface.h"
#endif

#include <cassert>

#include <QMessageBox>
#include <QString>

FlashWindow::FlashWindow(FlashablePtr flashable)
    : ui(new Ui::FlashWindow), flashable_(flashable) {
  assert(flashable);
  assert(flashable->valid());

  ui->setupUi(this);
  setWindowFlags(Qt::Window);

  layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void FlashWindow::on_buttonCancel_clicked() { close(); }

void FlashWindow::on_buttonFlash_clicked() {
  switch (ui->comboMode->currentIndex()) {
#ifdef WITH_SOCKETCAN
  case 0: {
    // SocketCAN
    std::shared_ptr<SocketCanInterface> can;
    try {
      can = SocketCanInterface::create(ui->editSocketCAN->text().toStdString());
    } catch (const std::exception &e) {
      QMessageBox msgBox;
      msgBox.setWindowTitle("SocketCan error");
      msgBox.setText("Could not bind socketcan interface: " +
                     QString(e.what()));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      return;
    }
    int serverId = flashable_->definition()->definition()->serverId();
    flasher_ = Flasher::createT1(
        this, flashable_->definition()->definition()->key(), std::make_shared<isotp::Protocol>(can, isotp::Options{serverId, serverId + 8}));
    if (!flasher_) {
      // The interface should have called the downloadError callback
      return;
    }

    flasher_->flash(flashable_);
    can->start();

    ui->stackedWidget->setCurrentIndex(1);
    ui->progressBar->setValue(0);

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

void FlashWindow::mainCompletion() {
  // flasher_.reset();
  QMessageBox msgBox;
  msgBox.setWindowTitle("Flash completed");
  msgBox.setText("Successfully flashed ROM");
  msgBox.setIcon(QMessageBox::Information);
  msgBox.exec();
}

void FlashWindow::onCompletion() {
  QMetaObject::invokeMethod(this, "mainCompletion", Qt::QueuedConnection);
}

void FlashWindow::mainError(const QString &error) {
  // flasher_.reset();
  QMessageBox msgBox;
  msgBox.setText("Error while flashing ROM: " + error);
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.setWindowTitle("Flash error");
  // msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.exec();

  ui->stackedWidget->setCurrentIndex(0);
}

void FlashWindow::onError(const std::string &error) {
  QMetaObject::invokeMethod(this, "mainError", Qt::QueuedConnection,
                            Q_ARG(QString, QString::fromStdString(error)));
}

void FlashWindow::onProgress(double percent) {
  QMetaObject::invokeMethod(ui->progressBar, "setValue", Qt::QueuedConnection,
                            Q_ARG(int, (int)(percent * 100)));
}
