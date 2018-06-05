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

#ifndef DOWNLOADWINDOW_H
#define DOWNLOADWINDOW_H

#include <QWidget>

#include <memory>

#include "downloadinterface.h"
#include "rommanager.h"

namespace Ui {
class DownloadWindow;
}

/**
 * Window for downloading firmware from the ECU
 */
class DownloadWindow : public QWidget, public DownloadInterface::Callbacks {
  Q_OBJECT
public:
  explicit DownloadWindow(QWidget *parent = 0);
  ~DownloadWindow() override;

  /* Download interface callbacks */
  void downloadError(const QString &error) override;
  void onCompletion(gsl::span<const uint8_t> data) override;
  void updateProgress(float progress) override;

private slots:
  void on_comboMode_activated(int index);
  void on_buttonContinue_clicked();
  void on_buttonBack_clicked();
  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent *event) override;

  void mainDownloadError(const QString &error);
  void mainOnCompletion(bool success, const QString &error);

private:
  Ui::DownloadWindow *ui;
  std::shared_ptr<DownloadInterface> downloadInterface_;
  std::string name_;
  DefinitionPtr definition_;

  void start();
};

#endif // DOWNLOADWINDOW_H
