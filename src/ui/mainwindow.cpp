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
#include "dataloggerwindow.h"
#include "flowlayout.h"
#include "logview.h"
#include "titlebar.h"
#include "sidebarwidget.h"
#include "tableswidget.h"
#include "editorwidget.h"
#include "tunedialog.h"
#include "graphwidget.h"
#include "setupdialog.h"
#include "rom.h"
#include "createtunedialog.h"

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


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), datalinksWindow_(LT()->datalinks()) {
    resize(QSize(1100, 630));

    setWindowTitle("LibreTuner");
    
    setDocumentMode(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    

    //main_ = new QMainWindow;
    //layout_->addWidget(main_);

    setupMenu();
    setupStatusBar();

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
    QByteArray geo = settings.value("mainwindow/geometry", QByteArray()).toByteArray();
    restoreGeometry(geo);
    QByteArray state = settings.value("mainwindow/state", QByteArray()).toByteArray();
    restoreState(state);
    QPoint pos = settings.value("mainwindow/pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("mainwindow/size", QSize(900, 600)).toSize();
    resize(size);
    move(pos);
}



void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("mainwindow/geometry", saveGeometry());
    settings.setValue("mainwindow/size", size());
    settings.setValue("mainwindow/pos", pos());
    settings.setValue("mainwindow/state", saveState());
}



QDockWidget *MainWindow::createLoggingDock() {
    QDockWidget *dock = new QDockWidget("Logging", this);
    dock->setObjectName("logging");
    auto *widget = new QWidget;
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
    
    connect(this, &MainWindow::tableChanged, sidebar_, &SidebarWidget::fillTableInfo);
    
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
        
        emit tableChanged(table);
    });
    docks_.emplace_back(dock);
    return dock;
}



QDockWidget *MainWindow::createEditorDock()
{
    QDockWidget *dock = new QDockWidget("Editor", this);
    dock->setObjectName("editor");
    editor_ = new EditorWidget(dock);
    
    connect(this, &MainWindow::tableChanged, editor_, &EditorWidget::tableChanged);
    
    dock->setWidget(editor_);
    docks_.emplace_back(dock);
    return dock;
}

bool MainWindow::changeSelected(Tune *tune)
{
    
    if (checkSaveSelected()) {
        try {
            if (tune) {
                selectedTune_ = tune->data();
                flashCurrentAction_->setEnabled(true);
                saveCurrentAction_->setEnabled(true);
                tables_->setTables(tune->base()->platform()->tables);
            } else {
                tables_->setTables(std::vector<definition::Table>());
                flashCurrentAction_->setEnabled(false);
                saveCurrentAction_->setEnabled(false);
                // editor_->setModel(nullptr);
                emit tableChanged(nullptr);
            }
        } catch (const std::runtime_error &err) {
            QMessageBox(QMessageBox::Critical, "Error", QString("Failed to load tune\n") + err.what()).exec();
        }
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
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
        default:
            return false;
        }
    }
    return true;
}



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
    
    connect(this, &MainWindow::tableChanged, graph_, &GraphWidget::tableChanged);
    
    docks_.emplace_back(dock);
    return dock;
}



void MainWindow::setupMenu() {
    auto *menuBar = new QMenuBar;
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    QMenu *viewMenu = menuBar->addMenu(tr("&View"));
    QMenu *toolsMenu = menuBar->addMenu(tr("&Tools"));
    
    auto *openTuneAction = new QAction(tr("&Open Tune"), this);
    fileMenu->addAction(openTuneAction);

    auto *createTuneAction = new QAction(tr("&New Tune"), this);
    fileMenu->addAction(createTuneAction);

    auto *downloadAction = new QAction(tr("&Download ROM"), this);
    fileMenu->addAction(downloadAction);
    
    saveCurrentAction_ = new QAction(tr("&Save Current Tune"), this);
    saveCurrentAction_->setEnabled(false);
    fileMenu->addAction(saveCurrentAction_);
    
    flashCurrentAction_ = new QAction(tr("Flash Current Tune"), this);
    flashCurrentAction_->setEnabled(false);
    fileMenu->addAction(flashCurrentAction_);
    
    connect(flashCurrentAction_, &QAction::triggered, [this]() {
        if (selectedTune_) {
            LibreTuner::get()->flashTune(selectedTune_);
        }
    });

    connect(createTuneAction, &QAction::triggered, []() {
        CreateTuneDialog dlg;
        dlg.exec();
    });
    
    connect(saveCurrentAction_, &QAction::triggered, [this]() {
        if (selectedTune_) {
            try {
                selectedTune_->save();
            } catch (const std::runtime_error &err) {
                QMessageBox::critical(this, "Error Saving", err.what());
            }
        }
    });
    
    connect(downloadAction, &QAction::triggered, this, &MainWindow::on_buttonDownloadRom_clicked);
    
    connect(openTuneAction, &QAction::triggered, [this]() {
        TuneDialog dlg;
        dlg.exec();
        
        Tune *tune = dlg.selectedTune();
        if (tune)
            changeSelected(tune);
    });

    QAction *logAct = toolsMenu->addAction(tr("&CAN Log"));
    connect(logAct, &QAction::triggered, [this] { canViewer_.show(); });

    QAction *datalinksAction = toolsMenu->addAction(tr("Setup &Datalinks"));
    connect(datalinksAction, &QAction::triggered, [this]() {
        datalinksWindow_.show();
    });

    /*auto *setupAction = toolsMenu->addAction(tr("Run &Setup"));
    connect(setupAction, &QAction::triggered, [this]() {
       LT()->setup();
    });*/

    setMenuBar(menuBar);
}


Q_DECLARE_METATYPE(definition::MainPtr);
Q_DECLARE_METATYPE(datalink::Link*);

void MainWindow::setupStatusBar() {
    auto *comboPlatform = new QComboBox;
    comboPlatform->setModel(DefinitionManager::get());
    connect(comboPlatform, QOverload<int>::of(&QComboBox::currentIndexChanged), [comboPlatform](int index) {
        QVariant var = comboPlatform->currentData(Qt::UserRole);
        if (!var.canConvert<definition::MainPtr>()) {
            return;
        }
        LT()->setPlatform(var.value<definition::MainPtr>());
    });

    auto *comboDatalink = new QComboBox;
    comboDatalink->setModel(&LT()->datalinks());
    connect(comboDatalink, QOverload<int>::of(&QComboBox::currentIndexChanged), [comboDatalink](int index) {
        QVariant var = comboDatalink->currentData(Qt::UserRole);
        if (!var.canConvert<datalink::Link*>()) {
            return;
        }

        LT()->setDatalink(var.value<datalink::Link*>());
    });

    if (LT()->platform()) {
        comboPlatform->setCurrentText(QString::fromStdString(LT()->platform()->name));
    }
    if (LT()->datalink()) {
        comboDatalink->setCurrentText(QString::fromStdString(LT()->datalink()->name()));
    }

    statusBar()->addPermanentWidget(comboPlatform);
    statusBar()->addPermanentWidget(comboDatalink);
}


void MainWindow::on_buttonDownloadRom_clicked() {
    if (downloadWindow_) {
        delete downloadWindow_;
        downloadWindow_ = nullptr;
    }

    DownloadWindow downloadWindow;
    downloadWindow.exec();
}



void MainWindow::newLogClicked() {
    auto *window = new DataLoggerWindow;
    window->setWindowModality(Qt::WindowModal);
    window->show();
}



void MainWindow::closeEvent(QCloseEvent *event) {
    if (!checkSaveSelected()) {
        event->ignore();
        return;
    }
    canViewer_.close();
    datalinksWindow_.close();
    
    saveSettings();
}
