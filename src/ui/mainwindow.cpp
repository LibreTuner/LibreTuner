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
#include "libretuner.h"

#include "datalog.h"

#include "diagnosticswidget.h"
#include "flowlayout.h"
#include "logview.h"
#include "romwidget.h"
#include "titlebar.h"
#include "tunemanager.h"
#include "tunewidget.h"
#include "sidebarwidget.h"
#include "romswidget.h"
#include "tableswidget.h"
#include "editorwidget.h"

#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QWindowStateChangeEvent>

#include <future>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    resize(QSize(1100, 630));

    setWindowTitle("LibreTuner");

    //main_ = new QMainWindow;
    //layout_->addWidget(main_);

    setupMenu();

    // Blank central widget
    QLabel *central = new QLabel("TEST");
    setCentralWidget(central);
    central->hide();

    setDockOptions(dockOptions() | QMainWindow::AllowNestedDocks);
    setDocumentMode(true);

    // Create docks
    logDock_ = createLogDock();
    overviewDock_ = createOverviewDock();
    romsDock_ = createRomsDock();
    loggingDock_ = createLoggingDock();
    diagnosticsDock_ = createDiagnosticsDock();
    sidebarDock_ = createSidebarDock();
    tablesDock_ = createTablesDock();
    editorDock_ = createEditorDock();

    // Setup corners
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

    // Place docks

    // Bottom
    addDockWidget(Qt::BottomDockWidgetArea, logDock_);

    // Roms | Central | Sidebar
    addDockWidget(Qt::TopDockWidgetArea, romsDock_);
    splitDockWidget(romsDock_, overviewDock_, Qt::Horizontal);
    splitDockWidget(overviewDock_, sidebarDock_, Qt::Horizontal);

    // Left
    splitDockWidget(romsDock_, tablesDock_, Qt::Vertical);

    // Top (central)

    tabifyDockWidget(overviewDock_, loggingDock_);
    tabifyDockWidget(overviewDock_, diagnosticsDock_);
    tabifyDockWidget(overviewDock_, editorDock_);
}



QDockWidget *MainWindow::createLoggingDock() {
    QDockWidget *dock = new QDockWidget("Logging", this);
    QWidget *widget = new QWidget;
    auto *layout = new QVBoxLayout();

    auto *hlayout = new QHBoxLayout();
    layout->addLayout(hlayout);

    QPushButton *buttonNewLog = new QPushButton(tr("New Log"));
    hlayout->addWidget(buttonNewLog);
    connect(buttonNewLog, &QPushButton::clicked, this,
            &MainWindow::newLogClicked);

    comboLogVehicles_ = new QComboBox();
    QSizePolicy policy = comboLogVehicles_->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    comboLogVehicles_->setSizePolicy(policy);
    hlayout->addWidget(comboLogVehicles_);

    listLogs_ = new QListView;
    layout->addWidget(listLogs_);

    widget->setLayout(layout);
    dock->setWidget(widget);
    return dock;
}



QDockWidget *MainWindow::createDiagnosticsDock() {
    QDockWidget *dock = new QDockWidget("Diagnostics", this);
    dock->setWidget(new DiagnosticsWidget);
    return dock;
}



QDockWidget *MainWindow::createLogDock() {
    LogView *log = new LogView;
    log->setModel(&LibreTuner::get()->log());

    QDockWidget *dock = new QDockWidget("Log", this);
    dock->setWidget(log);
    return dock;
}



QDockWidget *MainWindow::createSidebarDock()
{
    QDockWidget *dock = new QDockWidget("Sidebar", this);
    dock->setWidget(new SidebarWidget);
    return dock;
}



QDockWidget *MainWindow::createTablesDock()
{
    QDockWidget *dock = new QDockWidget("Tables");
    tables_ = new TablesWidget(dock);
    dock->setWidget(tables_);
    return dock;
}



QDockWidget *MainWindow::createEditorDock()
{
    QDockWidget *dock = new QDockWidget("Editor");
    editor_ = new EditorWidget(dock);
    dock->setWidget(editor_);
    return dock;
}



QDockWidget *MainWindow::createRomsDock() {
    QDockWidget *dock = new QDockWidget("ROMs", this);

    RomsWidget *roms = new RomsWidget(dock);
    RomsModel *model = new RomsModel(roms);
    model->setRoms(RomStore::get());
    model->setTunes(TuneManager::get());
    roms->setModel(model);
    dock->setWidget(roms);

    return dock;
}


QDockWidget *MainWindow::createOverviewDock() { return new QDockWidget("Overview", this); }



void MainWindow::setupMenu() {
    auto *menuBar = new QMenuBar;
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    QMenu *viewMenu = menuBar->addMenu(tr("&View"));

    QAction *logAct = viewMenu->addAction(tr("CAN Log"));
    connect(logAct, &QAction::triggered, [this] { canViewer_.show(); });

    QAction *interfacesAct = viewMenu->addAction(tr("Interfaces"));
    connect(interfacesAct, &QAction::triggered,
            [this] { interfacesWindow_.show(); });
    setMenuBar(menuBar);
}


void MainWindow::on_buttonDownloadRom_clicked() {
    if (downloadWindow_) {
        delete downloadWindow_;
        downloadWindow_ = nullptr;
    }

    if (const auto &link = LibreTuner::get()->getVehicleLink()) {
        auto di = link->downloader();
        downloadWindow_ =
            new DownloadWindow(std::move(di), link->vehicle(), this);
        downloadWindow_->show();
    }
}



void MainWindow::newLogClicked() {
    std::unique_ptr<VehicleLink> link = LibreTuner::get()->getVehicleLink();
    if (!link) {
        return;
    }

    std::unique_ptr<DataLogger> logger;

    try {
        logger = link->logger();
        if (!logger) {
            QMessageBox(QMessageBox::Critical, "Logger error",
                        "Failed to create a usable datalogger. The datalink "
                        "may not support the needed protocol or there is no "
                        "log mode set in the definition file.")
                .exec();
            return;
        }
    } catch (const std::exception &e) {
        QMessageBox(QMessageBox::Critical, "Logger error",
                    QStringLiteral(
                        "An error occurred while creating the datalogger: ") +
                        e.what())
            .exec();
        return;
    }
    DataLogPtr log = std::make_shared<DataLog>();
    loggerWindow_ = std::make_unique<DataLoggerWindow>(
        log, std::move(logger), link->vehicle().definition);
    loggerWindow_->show();
}



void MainWindow::closeEvent(QCloseEvent * /*event*/) {
    canViewer_.close();
    interfacesWindow_.close();
}
