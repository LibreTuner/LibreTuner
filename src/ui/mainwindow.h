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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "canviewer.h"
#include "downloadwindow.h"
#include "interfaceswindow.h"
#include "styledwindow.h"
#include "dataloggerwindow.h"

#include <QMainWindow>
#include <QLayout>
#include <QComboBox>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);

private slots:
  void on_buttonDownloadRom_clicked();
  void newLogClicked();
  void closeEvent(QCloseEvent *event) override;

private:
  CanViewer canViewer_;
  DownloadWindow *downloadWindow_ = nullptr;
  InterfacesWindow interfacesWindow_;
  DataLoggerWindow loggerWindow_;

  QLayout *tunesLayout_;
  QLayout *romsLayout_;
  QComboBox *comboLogVehicles_;
  QListView *listLogs_;

  void setupMenu();

  QWidget *createOverviewTab();
  QWidget *createTunesTab();
  QWidget *createRomsTab();
  QWidget *createLogsTab();

public slots:
  void updateRoms();
  void updateTunes();
};

#endif // MAINWINDOW_H
