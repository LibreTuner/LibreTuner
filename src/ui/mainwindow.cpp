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

#include "libretuner.h"
#include "mainwindow.h"

#include "flowlayout.h"
#include "romwidget.h"
#include "tunemanager.h"
#include "tunewidget.h"
#include "titlebar.h"

#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QScrollArea>
#include <QWindowStateChangeEvent>


MainWindow::MainWindow(QWidget *parent)
    : StyledWindow(parent) {
  resize(QSize(1100, 630));

  setupWindow();
  setupMenu();

  QTabWidget *tabs = new QTabWidget();
  tabs->addTab(createOverviewTab(), "Overview");
  tabs->addTab(createTunesTab(), "Tunes");
  tabs->addTab(createRomsTab(), "ROMs");
  tabs->addTab(createLogsTab(), "Logs");

  mainWindow_->setCentralWidget(tabs);

  connect(RomManager::get(), &RomManager::updateRoms, this,
          &MainWindow::updateRoms);
  connect(TuneManager::get(), &TuneManager::updateTunes, this,
          &MainWindow::updateTunes);
  updateRoms();
  updateTunes();
}


QWidget *MainWindow::createLogsTab() {
  QWidget *widget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout();

  QHBoxLayout *hlayout = new QHBoxLayout();
  layout->addLayout(hlayout);

  QPushButton *buttonNewLog = new QPushButton(tr("New Log"));
  hlayout->addWidget(buttonNewLog);
  comboLogVehicles_ = new QComboBox();
  QSizePolicy policy = comboLogVehicles_->sizePolicy();
  policy.setHorizontalPolicy(QSizePolicy::Expanding);
  comboLogVehicles_->setSizePolicy(policy);
  hlayout->addWidget(comboLogVehicles_);

  listLogs_ = new QListView;
  layout->addWidget(listLogs_);

  widget->setLayout(layout);
  return widget;
}


QWidget *MainWindow::createRomsTab() {
  QWidget *widget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout();

  QScrollArea *area = new QScrollArea;
  area->setWidgetResizable(true);

  area->setStyleSheet("QScrollArea { background: transparent; }\n"
                      "QScrollArea > QWidget > QWidget { background: transparent; }\n"
                      "QScrollArea > QWidget > QScrollBar { background: palette(base); }");
  layout->addWidget(area);

  QWidget *scrollContents = new QWidget();
  romsLayout_ = new FlowLayout();
  scrollContents->setLayout(romsLayout_);
  area->setWidget(scrollContents);

  QPushButton *buttonDownload = new QPushButton(tr("Download new ROM"));
  connect(buttonDownload, &QPushButton::clicked, this, &MainWindow::on_buttonDownloadRom_clicked);
  layout->addWidget(buttonDownload);

  widget->setLayout(layout);
  return widget;
}


QWidget *MainWindow::createTunesTab() {
  QWidget *widget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout();

  QScrollArea *area = new QScrollArea;
  area->setWidgetResizable(true);
  area->setStyleSheet("QScrollArea { background: transparent; }\n"
                      "QScrollArea > QWidget > QWidget { background: transparent; }\n"
                      "QScrollArea > QWidget > QScrollBar { background: palette(base); }");
  layout->addWidget(area);

  QWidget *scrollContents = new QWidget();
  tunesLayout_ = new FlowLayout();
  scrollContents->setLayout(tunesLayout_);
  area->setWidget(scrollContents);

  QPushButton *buttonCreateTune = new QPushButton(tr("Create new tune"));
  layout->addWidget(buttonCreateTune);

  widget->setLayout(layout);
  return widget;
}


QWidget *MainWindow::createOverviewTab() {
  return new QWidget();
}


void MainWindow::setupMenu() {
  QMenuBar *menuBar = new QMenuBar;
  QMenu *fileMenu = menuBar->addMenu(tr("&File"));
  QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
  QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
  QMenu *viewMenu = menuBar->addMenu(tr("&View"));

  QAction *logAct = viewMenu->addAction(tr("CAN Log"));
  connect(logAct, &QAction::triggered, [this] { canViewer_.show(); });

  QAction *interfacesAct = viewMenu->addAction(tr("Interfaces"));
  connect(interfacesAct, &QAction::triggered, [this] { interfacesWindow_.show(); });

  mainWindow_->setMenuBar(menuBar);
}


void MainWindow::setupWindow()
{
    mainWindow_ = new QMainWindow;
    mainLayout()->addWidget(mainWindow_);
}


void MainWindow::updateTunes() {
  QLayoutItem *child;
  while ((child = tunesLayout_->takeAt(0)) != 0) {
    delete child;
  }

  for (const TunePtr &tune : TuneManager::get()->tunes()) {
    tunesLayout_->addWidget(new TuneWidget(tune));
  }
}


void MainWindow::updateRoms() {
  QLayoutItem *child;
  while ((child = romsLayout_->takeAt(0)) != 0) {
    delete child;
  }

  for (const RomPtr &rom : RomManager::get()->roms()) {
    romsLayout_->addWidget(new RomWidget(rom));
  }
}


void MainWindow::on_buttonDownloadRom_clicked() {
  DataLinkPtr link = LibreTuner::get()->getDataLink();
  if (link) {
    downloadWindow_ = new DownloadWindow(link, this);
    downloadWindow_->setAttribute(Qt::WA_DeleteOnClose, true);
    downloadWindow_->show();
  }
}


void MainWindow::closeEvent(QCloseEvent *event) {
  canViewer_.close();
  interfacesWindow_.close();
}
