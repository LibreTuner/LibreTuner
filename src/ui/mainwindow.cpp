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
#include "titlebar.h"
#include "sidebarwidget.h"
#include "tableswidget.h"
#include "editorwidget.h"
#include "tunedialog.h"
#include "graphwidget.h"

#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QWindowStateChangeEvent>
#include <QSettings>

#include <future>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    resize(QSize(1100, 630));

    setWindowTitle("LibreTuner");
    
    setDocumentMode(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    

    //main_ = new QMainWindow;
    //layout_->addWidget(main_);

    setupMenu();

    // Blank central widget
    //QLabel *central = new QLabel("TEST");
    //setCentralWidget(central);
    //central->hide();

    setDockOptions(dockOptions() | QMainWindow::AllowNestedDocks);
    setDocumentMode(true);

    // Create docks
    logDock_ = createLogDock();
    overviewDock_ = createOverviewDock();
    loggingDock_ = createLoggingDock();
    diagnosticsDock_ = createDiagnosticsDock();
    sidebarDock_ = createSidebarDock();
    tablesDock_ = createTablesDock();
    editorDock_ = createEditorDock();
    graphDock_ = createGraphDock();

    // Setup corners
    /*setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);*/
    
    
    //hideAllDocks();
    restoreDocks();
    
    loadSettings();
}


void MainWindow::hideAllDocks()
{
    for (auto dock : docks_) {
        removeDockWidget(dock);
    }
}



void MainWindow::restoreDocks()
{
    // Place docks

    // Roms | Central | Sidebar
    addDockWidget(Qt::TopDockWidgetArea, tablesDock_);
    splitDockWidget(tablesDock_, overviewDock_, Qt::Horizontal);
    splitDockWidget(overviewDock_, sidebarDock_, Qt::Horizontal);
    
    // Bottom
    addDockWidget(Qt::BottomDockWidgetArea, logDock_);
    
    // Top (central)

    tabifyDockWidget(overviewDock_, loggingDock_);
    tabifyDockWidget(overviewDock_, diagnosticsDock_);
    tabifyDockWidget(overviewDock_, editorDock_);
    tabifyDockWidget(overviewDock_, graphDock_);
}


void MainWindow::loadSettings()
{
    QSettings settings;
    QByteArray geo = settings.value("geometry", QByteArray()).toByteArray();
    restoreGeometry(geo);
    QByteArray state = settings.value("state", QByteArray()).toByteArray();
    restoreState(state);
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}



void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("state", saveState());
}



QDockWidget *MainWindow::createLoggingDock() {
    QDockWidget *dock = new QDockWidget("Logging", this);
    dock->setObjectName("logging");
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
    docks_.emplace_back(dock);
    return dock;
}



QDockWidget *MainWindow::createDiagnosticsDock() {
    QDockWidget *dock = new QDockWidget("Diagnostics", this);
    dock->setObjectName("diagnostics");
    dock->setWidget(new DiagnosticsWidget);
    docks_.emplace_back(dock);
    return dock;
}



QDockWidget *MainWindow::createLogDock() {
    LogView *log = new LogView;
    log->setModel(&LibreTuner::get()->log());

    QDockWidget *dock = new QDockWidget("Log", this);
    dock->setObjectName("log");
    dock->setWidget(log);
    dock->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    docks_.emplace_back(dock);
    return dock;
}



QDockWidget *MainWindow::createSidebarDock()
{
    QDockWidget *dock = new QDockWidget("Sidebar", this);
    dock->setObjectName("dock");
    dock->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    
    sidebar_ = new SidebarWidget;
    sidebar_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    dock->setWidget(sidebar_);
    docks_.emplace_back(dock);
    return dock;
}



QDockWidget *MainWindow::createTablesDock()
{
    QDockWidget *dock = new QDockWidget("Tables", this);
    dock->setObjectName("tables");
    tables_ = new TablesWidget(dock);
    dock->setWidget(tables_);
    
    connect(tables_, &TablesWidget::activated, [this](int index) {
        Table *table = selectedTune_->tables().get(index, true);
        
        sidebar_->fillTableInfo(table); 
        editor_->tableChanged(table);
        graph_->tableChanged(table);
    });
    docks_.emplace_back(dock);
    return dock;
}



QDockWidget *MainWindow::createEditorDock()
{
    QDockWidget *dock = new QDockWidget("Editor", this);
    dock->setObjectName("editor");
    editor_ = new EditorWidget(dock);
    dock->setWidget(editor_);
    docks_.emplace_back(dock);
    return dock;
}

bool MainWindow::changeSelected(const std::shared_ptr<TuneData>& data)
{
    if (checkSaveSelected()) {
        selectedTune_ = data;
        return true;
    }
    return false;
}



bool MainWindow::checkSaveSelected()
{
    if (!selectedTune_) {
        return true;
    }
    
    if (selectedTune_->dirty()) {
        QMessageBox mb;
        mb.setText(tr("This tune has been modified"));
        mb.setWindowTitle(tr("Unsaved changes"));
        mb.setInformativeText(tr("Do you want to save your changes?"));
        mb.setIcon(QMessageBox::Question);
        mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Discard |
                              QMessageBox::Save);
        mb.setDefaultButton(QMessageBox::Save);
        switch (mb.exec()) {
        case QMessageBox::Save:
            // Save then accept
            try {  
                selectedTune_->save();
                return true;
            } catch (const std::runtime_error &err) {
                QMessageBox msg;
                msg.setWindowTitle(tr("Error while saving tune"));
                msg.setText(tr(err.what()));
                msg.setIcon(QMessageBox::Critical);
                msg.exec();
                return false;
            }
            
            return false;
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
        default:
            return false;
        }
    }
    return true;
}


/*
QDockWidget *MainWindow::createRomsDock() {
    QDockWidget *dock = new QDockWidget("ROMs", this);

    RomsView *roms = new RomsView (dock);
    RomsModel *model = new RomsModel(roms);
    model->setRoms(RomStore::get());
    roms->setModel(model);
    dock->setWidget(roms);
    
    
    connect(roms, &RomsView::activated, [this](const std::shared_ptr<Tune> &tune) {
        if (!tune) {
            if (changeSelected(std::shared_ptr<TuneData>())) 
                tables_->setTables(std::vector<definition::Table>());
            return;
        }
        
        if (changeSelected(LibreTuner::openTune(tune)))
            tables_->setTables(tune->base()->platform()->tables);
    });
    
    connect(roms, &RomsView::downloadClicked, this, &MainWindow::on_buttonDownloadRom_clicked);
    
    connect(roms, &RomsWidget::flashClicked, [this]() {
        if (selectedTune_) {
            LibreTuner::get()->flashTune(selectedTune_);
        }
    });
    
    docks_.emplace_back(dock);
    return dock;
}*/


QDockWidget *MainWindow::createOverviewDock() {
    QDockWidget *dock = new QDockWidget("Overview", this);
    dock->setObjectName("overview");
    docks_.emplace_back(dock);
    return dock;
}



QDockWidget *MainWindow::createGraphDock()
{
    QDockWidget *dock = new QDockWidget("Graph", this);
    
    graph_ = new GraphWidget(dock);
    dock->setWidget(graph_);
    
    dock->setObjectName("graph");
    docks_.emplace_back(dock);
    return dock;
}



void MainWindow::setupMenu() {
    auto *menuBar = new QMenuBar;
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    QMenu *viewMenu = menuBar->addMenu(tr("&View"));
    
    QAction *openTuneAction = new QAction(tr("&Open Tune"), this);
    fileMenu->addAction(openTuneAction);
    
    QAction *downloadAction = new QAction(tr("&Download ROM"), this);
    fileMenu->addAction(downloadAction);
    
    connect(downloadAction, &QAction::triggered, this, &MainWindow::on_buttonDownloadRom_clicked);
    
    connect(openTuneAction, &QAction::triggered, [this]() {
        TuneDialog dlg;
        dlg.exec();
        
        Tune *tune = dlg.selectedTune();
        if (tune != nullptr) {
            try {
                if (changeSelected(tune->data())) {
                    tables_->setTables(tune->base()->platform()->tables);
                } else {
                    tables_->setTables(std::vector<definition::Table>());
                    editor_->setModel(nullptr);
                }
            } catch (const std::runtime_error &err) {
                QMessageBox(QMessageBox::Critical, "Error", QString("Failed to load tune\n") + err.what()).exec();
            }
        }
    });

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
        try {
            auto di = link->downloader();
            downloadWindow_ =
                new DownloadWindow(std::move(di), link->vehicle(), this);
            downloadWindow_->show();
        } catch (const std::runtime_error &err) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Download error");
            msgBox.setText(QStringLiteral("Error downloading ROM\n") + err.what());
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
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



void MainWindow::closeEvent(QCloseEvent *event) {
    if (!checkSaveSelected()) {
        event->ignore();
        return;
    }
    canViewer_.close();
    interfacesWindow_.close();
    
    saveSettings();
}
