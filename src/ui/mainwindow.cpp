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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "flowlayout.h"
#include "romwidget.h"
#include "tunemanager.h"
#include "tunewidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      downloadWindow_(new DownloadWindow(this)) {
  ui->setupUi(this);

  connect(RomManager::get(), &RomManager::updateRoms, this,
          &MainWindow::updateRoms);
  connect(TuneManager::get(), &TuneManager::updateTunes, this,
          &MainWindow::updateTunes);
  updateRoms();
  updateTunes();
}

void MainWindow::updateTunes() {
  QLayoutItem *child;
  while ((child = ui->tuneLayout->takeAt(0)) != 0) {
    delete child;
  }

  for (TunePtr tune : TuneManager::get()->tunes()) {
    ui->tuneLayout->addWidget(new TuneWidget(tune));
  }
}

void MainWindow::updateRoms() {
  QLayoutItem *child;
  while ((child = ui->romLayout->takeAt(0)) != 0) {
    delete child;
  }

  for (RomPtr rom : RomManager::get()->roms()) {
    ui->romLayout->addWidget(new RomWidget(rom));
  }
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_actionCAN_Log_triggered() { canViewer_.show(); }

void MainWindow::on_buttonDownloadRom_clicked() { downloadWindow_->show(); }

void MainWindow::closeEvent(QCloseEvent *event) { canViewer_.close(); }
