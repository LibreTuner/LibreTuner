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

#include "datalog.h"

#include "flowlayout.h"
#include "romwidget.h"
#include "tunemanager.h"
#include "tunewidget.h"
#include "titlebar.h"
#include "logview.h"

#include <QDockWidget>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QScrollArea>
#include <QWindowStateChangeEvent>
#include <QMessageBox>

#include <future>


MainWindow::MainWindow(QWidget *parent)
    : StyledWindow(parent) {
  resize(QSize(1100, 630));

  setTitle("LibreTuner");

  main_ = new QMainWindow;
  layout_->addWidget(main_);

  setupMenu();

  auto *tabs = new QTabWidget();
  tabs->setDocumentMode(true);
  tabs->tabBar()->setDrawBase(false);
  tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  tabs->addTab(createOverviewTab(), "Overview");
  tabs->addTab(createTunesTab(), "Tunes");
  tabs->addTab(createRomsTab(), "ROMs");
  tabs->addTab(createLogsTab(), "Logs");
  tabs->addTab(createDiagnosticsTab(), "Diagnostics");

  createLog();

  main_->setCentralWidget(tabs);

  connect(RomManager::get(), &RomManager::updateRoms, this,
          &MainWindow::updateRoms);
  connect(TuneManager::get(), &TuneManager::updateTunes, this,
          &MainWindow::updateTunes);
  updateRoms();
  updateTunes();
}



QWidget *MainWindow::createLogsTab() {
  QWidget *widget = new QWidget();
  auto *layout = new QVBoxLayout();

  auto *hlayout = new QHBoxLayout();
  layout->addLayout(hlayout);

  QPushButton *buttonNewLog = new QPushButton(tr("New Log"));
  hlayout->addWidget(buttonNewLog);
  connect(buttonNewLog, &QPushButton::clicked, this, &MainWindow::newLogClicked);

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



QWidget *MainWindow::createDiagnosticsTab()
{
    QWidget *widget = new QWidget();
    auto *layout = new QVBoxLayout();
    QPushButton *buttonScan = new QPushButton(tr("Scan for diagnostic codes"));
    layout->addWidget(buttonScan);
    connect(buttonScan, &QPushButton::clicked, [] {
        // Stub
    });

    listCodes_ = new QListView;
    layout->addWidget(listCodes_);

    widget->setLayout(layout);
    return widget;
}

void MainWindow::createLog()
{
    LogView *log = new LogView;
    log->setModel(&LibreTuner::get()->log());
    log->setReadOnly(true);
    log->setWordWrapMode(QTextOption::NoWrap);

    logDock_ = new QDockWidget;
    logDock_->setWidget(log);
    logDock_->setFeatures(QDockWidget::DockWidgetMovable);
    logDock_->setWindowTitle("Log");
    main_->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, logDock_);
}



QWidget *MainWindow::createRomsTab() {
  QWidget *widget = new QWidget();
  auto *layout = new QVBoxLayout();

  auto *area = new QScrollArea;
  area->setWidgetResizable(true);

  area->setStyleSheet("QScrollArea { background: transparent; }\n"
                      "QScrollArea > QWidget > QWidget { background: transparent; }\n");
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
  auto *layout = new QVBoxLayout();

  auto *area = new QScrollArea;
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
  auto *menuBar = new QMenuBar;
  QMenu *fileMenu = menuBar->addMenu(tr("&File"));
  QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
  QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
  QMenu *viewMenu = menuBar->addMenu(tr("&View"));

  QAction *logAct = viewMenu->addAction(tr("CAN Log"));
  connect(logAct, &QAction::triggered, [this] { canViewer_.show(); });

  QAction *interfacesAct = viewMenu->addAction(tr("Interfaces"));
  connect(interfacesAct, &QAction::triggered, [this] { interfacesWindow_.show(); });
  main_->setMenuBar(menuBar);
}

void MainWindow::updateTunes() {
  QLayoutItem *child;
  while ((child = tunesLayout_->takeAt(0)) != nullptr) {
    delete child;
  }

  for (const TunePtr &tune : TuneManager::get()->tunes()) {
    tunesLayout_->addWidget(new TuneWidget(tune));
  }
}



void MainWindow::updateRoms() {
  QLayoutItem *child;
  while ((child = romsLayout_->takeAt(0)) != nullptr) {
    delete child;
  }

  for (const RomPtr &rom : RomManager::get()->roms()) {
    romsLayout_->addWidget(new RomWidget(rom));
  }
}



void MainWindow::on_buttonDownloadRom_clicked() {
    if (downloadWindow_) {
        delete downloadWindow_;
        downloadWindow_ = nullptr;
    }

    if (const auto &link = LibreTuner::get()->getVehicleLink()) {
        auto di = link->downloader();
        downloadWindow_ = new DownloadWindow(std::move(di), link->vehicle(), this);
        downloadWindow_->show();
    }
}



void MainWindow::newLogClicked()
{
    VehicleLinkPtr link = LibreTuner::get()->getVehicleLink();
    if (!link) {
        return;
    }

    std::unique_ptr<DataLogger> logger = link->logger();
    if (!logger) {
        QMessageBox(QMessageBox::Critical, "Logger error", "Failed to create a usable datalogger. The datalink may not support the needed protocol or there is no log mode set in the definition file.").exec();
        return;
    }
    DataLogPtr log = std::make_shared<DataLog>();
    loggerWindow_ = std::make_unique<DataLoggerWindow>(log, std::move(logger), link->vehicle().definition);
    loggerWindow_->show();
}



void MainWindow::closeEvent(QCloseEvent * /*event*/) {
  canViewer_.close();
  interfacesWindow_.close();
}
