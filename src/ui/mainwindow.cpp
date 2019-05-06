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

#include "downloadwindow.h"

#include "docks/graphwidget.h"
#include "docks/overviewwidget.h"
#include "docks/sidebarwidget.h"
#include "docks/tableswidget.h"
#include "docks/logview.h"
#include "docks/editorwidget.h"
#include "docks/diagnosticswidget.h"

#include "windows/definitionswindow.h"

#include "dataloggerwindow.h"
#include "createtunedialog.h"
#include "flasherwindow.h"
#include "uiutil.h"
#include "datalinkswidget.h"
#include "sessionscannerdialog.h"

#include "titlebar.h"

#include <QAction>
#include <QDockWidget>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStatusBar>
#include <QWindowStateChangeEvent>
#include <QFileDialog>

#include <future>
#include <lt/link/datalink.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), linksList_(LT()->links()) {
    resize(QSize(1100, 630));

    setWindowTitle("LibreTuner");

    setDocumentMode(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setupMenu();
    setupStatusBar();

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

    restoreDocks();

    loadSettings();
}

void MainWindow::saveTune(bool newPath)
{
    if (!tune_) {
        return;
    }

    if (tunePath_.empty() || newPath) {
        QString qPath = QFileDialog::getSaveFileName(this, tr("Save tune"), QString(), tr("Tune file (*.ltt)"));
        if (qPath.isNull()) {
            return;
        }

        tunePath_ = qPath.toStdString();
    } else if (!tune_->dirty()) {
        // If the tune is not dirty and we're not saving to a new location,
        // don't save.
        return;
    }

    LT()->saveTune(*tune_, tunePath_);
    tune_->clearDirty();
}

void MainWindow::hideAllDocks() {
    for (auto dock : docks_) {
        removeDockWidget(dock);
    }
}

void MainWindow::restoreDocks() {
    for (auto dock : docks_) {
        dock->show();
    }
    // Place docks

    // Roms | Central | Sidebar
    addDockWidget(Qt::TopDockWidgetArea, overviewDock_);
    splitDockWidget(overviewDock_, tablesDock_, Qt::Horizontal);
    splitDockWidget(tablesDock_, sidebarDock_, Qt::Vertical);

    // Bottom
    addDockWidget(Qt::BottomDockWidgetArea, logDock_);

    // Top (central)

    tabifyDockWidget(overviewDock_, loggingDock_);
    tabifyDockWidget(overviewDock_, diagnosticsDock_);
    tabifyDockWidget(overviewDock_, editorDock_);
    tabifyDockWidget(overviewDock_, graphDock_);
}

void MainWindow::loadSettings() {
    QSettings settings;
    QByteArray geo =
        settings.value("mainwindow/geometry", QByteArray()).toByteArray();
    restoreGeometry(geo);
    QByteArray state =
        settings.value("mainwindow/state", QByteArray()).toByteArray();
    restoreState(state);
    QPoint pos = settings.value("mainwindow/pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("mainwindow/size", QSize(900, 600)).toSize();
    resize(size);
    move(pos);
}

bool MainWindow::checkSave() {
    if (!tune_) {
        return true;
    }
    if (!tune_->dirty()) {
        return true;
    }

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
        catchCritical([this]() {
            saveTune();
            return true;
        }, tr("Error while saving tune"));
        return false;
    case QMessageBox::Discard:
        return true;
    case QMessageBox::Cancel:
    default:
        return false;
    }
}

void MainWindow::setTune(const lt::TunePtr &tune, const std::filesystem::path &path)
{
    // Save any previous tunes
    if (!checkSave()) {
        return;
    }

    tableModel_.setTable(nullptr);
    emit tableChanged(nullptr);

    tune_ = tune;
    tunePath_ = path;
    emit tuneChanged(tune_.get());

    flashCurrentAction_->setEnabled(!!tune);
    saveCurrentAction_->setEnabled(!!tune);

    if (tune) {
        setWindowTitle(tr("LibreTuner") + " - " + QString::fromStdString(tune->name()));
    } else {
        setWindowTitle(tr("LibreTuner"));
    }
}

void MainWindow::saveSettings() {
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
    log->setModel(&LT()->log());

    QDockWidget *dock = new QDockWidget("Log", this);
    dock->setObjectName("log");
    dock->setWidget(log);
    dock->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    docks_.emplace_back(dock);
    return dock;
}

QDockWidget *MainWindow::createSidebarDock() {
    QDockWidget *dock = new QDockWidget("Sidebar", this);
    dock->setObjectName("dock");
    dock->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);

    sidebar_ = new SidebarWidget;
    sidebar_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    dock->setWidget(sidebar_);

    docks_.emplace_back(dock);
    return dock;
}

void MainWindow::setTable(const lt::ModelTable *modTable)
{
    if (modTable == nullptr) {
        // Don't change
        return;
    }

    sidebar_->fillTableInfo(modTable);

    const lt::TableDefinition *tabDef = modTable->table;

    catchWarning([this, tabDef]() {
        lt::Table *tab = tune_->getTable(tabDef->id, true);
        tableModel_.setTable(tab);
        emit tableChanged(tab);
    }, tr("Error creating table"));
}

QDockWidget *MainWindow::createTablesDock() {
    QDockWidget *dock = new QDockWidget("Tables", this);
    dock->setObjectName("tables");
    tables_ = new TablesWidget(dock);
    dock->setWidget(tables_);
    
    connect(tables_, &TablesWidget::activated, this, &MainWindow::setTable);
    connect(this, &MainWindow::tuneChanged, [this](const lt::Tune *tune) {
        if (tune == nullptr) {
            return;
        }
        tables_->setModel(*tune->base()->model());
    });
    docks_.emplace_back(dock);
    return dock;
}

QDockWidget *MainWindow::createEditorDock() {
    QDockWidget *dock = new QDockWidget("Editor", this);
    dock->setObjectName("editor");
    editor_ = new EditorWidget(dock);
    editor_->setModel(&tableModel_);
    dock->setWidget(editor_);
    docks_.emplace_back(dock);
    return dock;
}

QDockWidget *MainWindow::createOverviewDock() {
    QDockWidget *dock = new QDockWidget("Overview", this);
    dock->setObjectName("overview");

    dock->setWidget(new OverviewWidget);

    docks_.emplace_back(dock);
    return dock;
}

QDockWidget *MainWindow::createGraphDock() {
    QDockWidget *dock = new QDockWidget("Graph", this);

    graph_ = new GraphWidget(dock);
    dock->setWidget(graph_);
    dock->setObjectName("graph");
    graph_->setModel(&tableModel_);

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

    // File menu
    auto *flashAction = fileMenu->addAction(tr("Flash Tune"));
    auto *openTuneAction = fileMenu->addAction(tr("&Open Tune"));
    openTuneAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    auto *createTuneAction = fileMenu->addAction(tr("&New Tune"));

    createTuneAction->setShortcut(QKeySequence(Qt::CTRL +Qt::Key_N));
    auto *downloadAction = fileMenu->addAction(tr("&Download ROM"));

    saveCurrentAction_ = fileMenu->addAction(tr("&Save Tune"));
    saveCurrentAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    saveCurrentAction_->setEnabled(false);

    flashCurrentAction_ = fileMenu->addAction(tr("Flash Current Tune"));
    flashCurrentAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    flashCurrentAction_->setEnabled(false);

    // View menu
    auto *openPlatformsAction = viewMenu->addAction(tr("Platforms"));
    connect(openPlatformsAction, &QAction::triggered, [this]() {
        if (!definitionsWindow_) {
            definitionsWindow_ = new DefinitionsWindow;
            definitionsWindow_->setDefinitions(&LT()->definitions());
        }
        definitionsWindow_->show();
    });

    QMenu *themeMenu = viewMenu->addMenu(tr("Theme"));

    auto *resetLayoutAction = viewMenu->addAction(tr("Reset Layout"));
    connect(resetLayoutAction, &QAction::triggered, [this]() {
        hideAllDocks();
        restoreDocks();
    });

    auto *nativeThemeAction = themeMenu->addAction(tr("Native"));

    connect(nativeThemeAction, &QAction::triggered,
            []() { LT()->setStyleSheet(""); });

    connect(flashAction, &QAction::triggered, []() {
        /*FlasherWindow flasher;
        flasher.exec();*/
    });

    connect(flashCurrentAction_, &QAction::triggered, [this]() {
        if (tune_) {
            FlasherWindow flasher;
            flasher.setTune(tune_);
            flasher.exec();
        }
    });

    connect(createTuneAction, &QAction::triggered, [this]() {
        // Check if an unsaved tune is in the workspace first
        if (!checkSave()) {
            return;
        }
        CreateTuneDialog dlg;
        dlg.exec();
        lt::TunePtr tune = dlg.tune();
        if (!tune) {
            return;
        }


        setTune(dlg.tune());
    });

    connect(saveCurrentAction_, &QAction::triggered, [this]() {
        catchCritical([this]() {
            saveTune();
        }, tr("Error saving tune"));
    });

    connect(downloadAction, &QAction::triggered, this,
            &MainWindow::on_buttonDownloadRom_clicked);

    connect(openTuneAction, &QAction::triggered, [this]() {
        // Check if we have an unsaved tune in the workspace
        if (!checkSave()) {
            return;
        }
        QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open Tune"), QString(), tr("Tune Files (*.ltt)"));
        if (fileName.isNull()) {
            return;
        }

        catchCritical([this, &fileName]() {
            std::filesystem::path path(fileName.toStdString());
            setTune(LT()->openTune(path), path);
        }, tr("Error opening tune"));
    });

    QAction *logAction = toolsMenu->addAction(tr("&CAN Log"));
    // connect(logAct, &QAction::triggered, [this] { canViewer_.show(); });

    QAction *datalinksAction = toolsMenu->addAction(tr("Setup &Datalinks"));
    connect(datalinksAction, &QAction::triggered, [this]() {
        datalinksWindow_.show();
    });

    auto *sessionScanAct = toolsMenu->addAction("Session Scanner");
    connect(sessionScanAct, &QAction::triggered, [this]() {
        SessionScannerDialog scanner;
        scanner.exec();
    });

    setMenuBar(menuBar);
}

void MainWindow::setupStatusBar() {
    auto *comboPlatform = new QComboBox;
    comboPlatform->setModel(&LT()->definitions());
    connect(comboPlatform, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [comboPlatform](int) {
                QVariant var = comboPlatform->currentData(Qt::UserRole);
                if (!var.canConvert<lt::PlatformPtr>()) {
                    return;
                }
                LT()->setPlatform(var.value<lt::PlatformPtr>());
            });

    comboDatalink_ = new QComboBox;
    comboDatalink_->setModel(&linksList_);

    connect(comboDatalink_, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int) {
            QVariant var = comboDatalink_->currentData(Qt::UserRole);
            if (!var.canConvert<lt::DataLink*>()) {
                return;
            }

            LT()->setDatalink(var.value<lt::DataLink*>());
        });

    if (LT()->platform()) {
        comboPlatform->setCurrentText(
            QString::fromStdString(LT()->platform()->name));
    }
    if (LT()->datalink()) {
        comboDatalink_->setCurrentText(
            QString::fromStdString(LT()->datalink()->name()));
    }

    statusBar()->addPermanentWidget(comboPlatform);
    statusBar()->addPermanentWidget(comboDatalink_);
}

void MainWindow::on_buttonDownloadRom_clicked() {
    /*if (downloadWindow_) {
        delete downloadWindow_;
        downloadWindow_ = nullptr;
    }*/

    DownloadWindow downloadWindow;
    downloadWindow.exec();
}

void MainWindow::newLogClicked() {
    auto *window = new DataLoggerWindow;
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowModality(Qt::WindowModal);
    window->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (!checkSave()) {
        event->ignore();
        return;
    }
    /*canViewer_.close();
    datalinksWindow_.close();*/

    saveSettings();
}
