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
#include "logger.h"

#include <cassert>

#include <QMessageBox>
#include <QString>
#include <QStyledItemDelegate>

FlashWindow::FlashWindow(std::unique_ptr<Flasher> &&flasher,
                         Flashable &&flashable)
    : ui(new Ui::FlashWindow), flashable_(std::move(flashable)),
      flasher_(std::move(flasher)) {
    assert(flasher_);

    ui->setupUi(this);
    
    setWindowTitle("Flash tune");
    
    ui->comboMode->setItemDelegate(new QStyledItemDelegate());

    flasher_->setProgressCallback(
        [this](float progress) { onProgress(progress); });
}



FlashWindow::~FlashWindow() { stop(); }



void FlashWindow::on_buttonCancel_clicked() { close(); }



void FlashWindow::on_buttonFlash_clicked() {
    if (worker_.joinable() || !flasher_) {
        // Already flashing
        return;
    }
    ui->stackedWidget->setCurrentIndex(1);
    worker_ = std::thread([this] {
        try {
            if (flasher_->flash(flashable_)) {
                onCompletion();
            } else {
                onError("canceled");
            }
        } catch (const std::exception &e) {
            onError(e.what());
        }
    });
}



void FlashWindow::mainCompletion() {
    // flasher_.reset();
    QMessageBox msgBox;
    msgBox.setWindowTitle("Flash completed");
    msgBox.setText("Successfully flashed ROM");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}



void FlashWindow::stop() {
    if (worker_.joinable()) {
        Logger::info("Canceling flash");
        flasher_->cancel();
        worker_.join();
    }
    flasher_.reset();
}



void FlashWindow::closeEvent(QCloseEvent *event) { stop(); }



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



void FlashWindow::onProgress(float percent) {
    QMetaObject::invokeMethod(ui->progressBar, "setValue", Qt::QueuedConnection,
                              Q_ARG(int, static_cast<int>(percent * 100)));
}
