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

#include "ui/windows/downloadwindow.h"
#include "ui/windows/vehicleinformationwidget.h"

#include "docks/editorwidget.h"
#include "docks/explorerwidget.h"
#include "docks/graphwidget.h"
#include "docks/logview.h"
#include "docks/overviewwidget.h"
#include "docks/sidebarwidget.h"
#include "docks/tableswidget.h"
#include "ui/windows/diagnosticswidget.h"
#include "widget/scalarview.h"

#include "windows/definitionswindow.h"

#include "datalinkswidget.h"
#include "dataloggerwindow.h"
#include "flasherwindow.h"
#include "sessionscannerdialog.h"
#include "ui/windows/createtunedialog.h"
#include "uiutil.h"

#include "titlebar.h"

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QListView>
#include <QMdiArea>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStatusBar>
#include <QWindowStateChangeEvent>

#include <database/definitions.h>
#include <future>
#include <lt/link/datalink.h>
#include <ui/widget/tableview.h>
#include <ui/windows/newprojectdialog.h>

MainWindow::MainWindow(QWidget * parent) : QMainWindow(parent), linksList_(LT()->links())
{
    // resize(QSize(1100, 630));
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

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
    sidebarDock_ = createSidebarDock();
    tablesDock_ = createTablesDock();
    explorerDock_ = createExplorerDock();

    restoreDocks();

    loadSettings();
}

void MainWindow::saveTune(bool newPath)
{
    if (!tune_)
        return;

    if (!tune_->dirty())
    {
        // If the tune is not dirty and we're not saving to a new location,
        // don't save.
        return;
    }

    tune_->save();
    tune_->clearDirty();
}

void MainWindow::hideAllDocks()
{
    for (auto dock : docks_)
    {
        removeDockWidget(dock);
    }
}

void MainWindow::restoreDocks()
{
    for (auto dock : docks_)
    {
        dock->show();
    }
    // Place docks

    // Roms | Central | Sidebar
    addDockWidget(Qt::TopDockWidgetArea, explorerDock_);
    splitDockWidget(explorerDock_, overviewDock_, Qt::Horizontal);
    splitDockWidget(overviewDock_, tablesDock_, Qt::Horizontal);
    splitDockWidget(tablesDock_, sidebarDock_, Qt::Vertical);

    // Bottom
    addDockWidget(Qt::BottomDockWidgetArea, logDock_);

    // Top (central)

    tabifyDockWidget(overviewDock_, loggingDock_);
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

    // Populate recent menu
    recentProjects_ = settings.value("projects").toStringList();
    for (const QString & path : recentProjects_)
    {
        addToRecentMenu(path);
    }
}

bool MainWindow::checkSave()
{
    if (!tune_)
        return true;
    if (!tune_->dirty())
        return true;

    QMessageBox mb;
    mb.setText(tr("This tune has been modified"));
    mb.setWindowTitle(tr("Unsaved changes"));
    mb.setInformativeText(tr("Do you want to save your changes?"));
    mb.setIcon(QMessageBox::Question);
    mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save);
    mb.setDefaultButton(QMessageBox::Save);
    switch (mb.exec())
    {
    case QMessageBox::Save:
        // Save then accept
        catchCritical(
            [this]() {
                saveTune();
                return true;
            },
            tr("Error while saving tune"));
        return false;
    case QMessageBox::Discard:
        return true;
    case QMessageBox::Cancel:
    default:
        return false;
    }
}

void MainWindow::setTune(const lt::TunePtr & tune)
{
    if (tune_ == tune)
        return;
    // Save any previous tunes
    if (!checkSave())
        return;

    for (auto & [id, view] : views_)
    {
        if (view)
            view->close();
    }

    tune_ = tune;
    emit tuneChanged(tune_.get());

    flashCurrentAction_->setEnabled(!!tune);
    saveCurrentAction_->setEnabled(!!tune);

    if (tune)
        setWindowTitle(tr("LibreTuner") + " - " + QString::fromStdString(tune->name()));
    else
        setWindowTitle(tr("LibreTuner"));
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("mainwindow/geometry", saveGeometry());
    settings.setValue("mainwindow/size", size());
    settings.setValue("mainwindow/pos", pos());
    settings.setValue("mainwindow/state", saveState());
    settings.setValue("projects", recentProjects_);
}

QDockWidget * MainWindow::createLoggingDock()
{
    QDockWidget * dock = new QDockWidget("Logging", this);
    dock->setObjectName("logging");
    auto * widget = new QWidget;
    auto * layout = new QVBoxLayout();

    auto * hlayout = new QHBoxLayout();
    layout->addLayout(hlayout);

    QPushButton * buttonNewLog = new QPushButton(tr("New Log"));
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
    dock->setWidget(widget);
    docks_.emplace_back(dock);
    return dock;
}

QDockWidget * MainWindow::createLogDock()
{
    LogView * log = new LogView;
    log->setModel(&LT()->log());

    QDockWidget * dock = new QDockWidget("Log", this);
    dock->setObjectName("log");
    dock->setWidget(log);
    dock->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    docks_.emplace_back(dock);
    return dock;
}

QDockWidget * MainWindow::createSidebarDock()
{
    QDockWidget * dock = new QDockWidget("Sidebar", this);
    dock->setObjectName("dock");
    dock->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);

    sidebar_ = new SidebarWidget;
    sidebar_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    dock->setWidget(sidebar_);

    docks_.emplace_back(dock);
    return dock;
}

void MainWindow::setTable(const lt::TableDefinition * table)
{
    if (table == nullptr)
    {
        // Don't change
        return;
    }

    sidebar_->fillTableInfo(table);

    if (auto it = views_.find(table->id); it != views_.end())
    {
        if (it->second)
        {
            it->second->show();
            return;
        }
    }

    catchWarning(
        [&]() {
            lt::Table * tab = tune_->getTable(table->id, true);
            if (tab == nullptr)
                return;
            if (tab->isScalar())
            {
                auto * view = new ScalarView;
                view->setTable(tab);
                view->setAttribute(Qt::WA_DeleteOnClose);
                view->setWindowFlag(Qt::WindowStaysOnTopHint);
                view->setWindowFlag(Qt::WindowMaximizeButtonHint, false);
                view->show();
                views_[table->id] = view;
            }
            else
            {
                auto * view = new TableView;
                view->resize(QGuiApplication::primaryScreen()->size() * 0.5);
                view->setTable(tab);
                view->setAttribute(Qt::WA_DeleteOnClose);
                view->setWindowFlag(Qt::WindowStaysOnTopHint);
                view->show();
                views_[table->id] = view;
            }

            // tableModel_.setTable(tab);
            // emit tableChanged(tab);
        },
        tr("Error creating table"));
}

QDockWidget * MainWindow::createTablesDock()
{
    QDockWidget * dock = new QDockWidget("Tables", this);
    dock->setObjectName("tables");
    tables_ = new TablesWidget(dock);
    dock->setWidget(tables_);

    connect(tables_, &TablesWidget::activated, this, &MainWindow::setTable);
    connect(this, &MainWindow::tuneChanged, [this](const lt::Tune * tune) {
        if (tune == nullptr)
            return;
        tables_->setModel(*tune->base()->model());
    });
    docks_.emplace_back(dock);
    return dock;
}

QDockWidget * MainWindow::createOverviewDock()
{
    QDockWidget * dock = new QDockWidget("Overview", this);
    dock->setObjectName("overview");

    dock->setWidget(new OverviewWidget);

    docks_.emplace_back(dock);
    return dock;
}

QDockWidget * MainWindow::createExplorerDock()
{
    QDockWidget * dock = new QDockWidget("Explorer", this);

    explorer_ = new ExplorerWidget(dock);
    explorer_->setModel(&LT()->projects());
    connect(explorer_->menu().actionNewProject(), &QAction::triggered, this, &MainWindow::newProject);
    connect(explorer_->menu().actionCreateTune(), &QAction::triggered, this, &MainWindow::openCreateTune);
    connect(explorer_, &ExplorerWidget::tuneOpened, this, &MainWindow::openTune);
    dock->setWidget(explorer_);
    dock->setObjectName("explorer");

    docks_.emplace_back(dock);
    return dock;
}

void MainWindow::setupMenu()
{
    auto * menuBar = new QMenuBar;
    QMenu * fileMenu = menuBar->addMenu(tr("&File"));
    QMenu * editMenu = menuBar->addMenu(tr("&Edit"));
    QMenu * helpMenu = menuBar->addMenu(tr("&Help"));
    QMenu * viewMenu = menuBar->addMenu(tr("&View"));
    QMenu * toolsMenu = menuBar->addMenu(tr("&Tools"));

    // File menu
    recentMenu_ = fileMenu->addMenu(tr("Recent Projects"));
    auto * flashAction = fileMenu->addAction(tr("Flash Tune"));
    auto * openTuneAction = fileMenu->addAction(tr("&Open Tune"));
    openTuneAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    auto * createTuneAction = fileMenu->addAction(tr("&New Tune"));
    auto * newProjectAction = fileMenu->addAction(tr("New Project"));
    auto * openProjectAction = fileMenu->addAction(tr("Open Project"));

    createTuneAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    auto * downloadAction = fileMenu->addAction(tr("&Download ROM"));

    saveCurrentAction_ = fileMenu->addAction(tr("&Save Tune"));
    saveCurrentAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    saveCurrentAction_->setEnabled(false);

    flashCurrentAction_ = fileMenu->addAction(tr("Flash Current Tune"));
    flashCurrentAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    flashCurrentAction_->setEnabled(false);

    // View menu
    auto * openPlatformsAction = viewMenu->addAction(tr("Platforms"));
    connect(openPlatformsAction, &QAction::triggered, [this]() {
        if (!definitionsWindow_)
        {
            definitionsWindow_ = new DefinitionsWindow;
            definitionsWindow_->setDefinitions(&LT()->definitions());
        }
        definitionsWindow_->show();
    });

    QMenu * themeMenu = viewMenu->addMenu(tr("Theme"));

    auto * resetLayoutAction = viewMenu->addAction(tr("Reset Layout"));
    connect(resetLayoutAction, &QAction::triggered, [this]() {
        hideAllDocks();
        restoreDocks();
    });

    auto * nativeThemeAction = themeMenu->addAction(tr("Native"));

    connect(nativeThemeAction, &QAction::triggered, []() { LT()->setStyleSheet(""); });

    connect(flashAction, &QAction::triggered, []() {
        /*FlasherWindow flasher;
        flasher.exec();*/
    });

    connect(flashCurrentAction_, &QAction::triggered, [this]() {
        if (tune_)
        {
            FlasherWindow flasher;
            flasher.setTune(tune_);
            flasher.exec();
        }
    });

    connect(createTuneAction, &QAction::triggered, this, &MainWindow::openCreateTune);

    connect(saveCurrentAction_, &QAction::triggered,
            [this]() { catchCritical([this]() { saveTune(); }, tr("Error saving tune")); });

    connect(newProjectAction, &QAction::triggered, this, &MainWindow::newProject);

    connect(openProjectAction, &QAction::triggered, this, &MainWindow::openProject);

    connect(downloadAction, &QAction::triggered, this, &MainWindow::on_buttonDownloadRom_clicked);

    connect(openTuneAction, &QAction::triggered, [this]() {
        // Check if we have an unsaved tune in the workspace
        if (!checkSave())
        {
            return;
        }
        QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open Tune"), QString(), tr("Tune Files (*.ltt)"));
        if (fileName.isNull())
        {
            return;
        }

        catchCritical(
            [this, &fileName]() {
                std::filesystem::path path(fileName.toStdString());
                // setTune(LT()->roms().loadTune(path), path);
            },
            tr("Error opening tune"));
    });

    // Tools menu

    QAction * logAction = toolsMenu->addAction(tr("&CAN Log"));
    // connect(logAct, &QAction::triggered, [this] { canViewer_.show(); });
    QAction * infoAction = toolsMenu->addAction(tr("Vehicle Information"));
    connect(infoAction, &QAction::triggered, [this]() {
        if (infoWidget_ == nullptr)
        {
            infoWidget_ = new VehicleInformationWidget(this);
            infoWidget_->setWindowFlag(Qt::Window);
        }
        infoWidget_->show();
    });

    QAction * datalinksAction = toolsMenu->addAction(tr("Setup &Datalinks"));
    connect(datalinksAction, &QAction::triggered, [this]() { datalinksWindow_.show(); });

    auto * sessionScanAct = toolsMenu->addAction(tr("Session Scanner"));
    connect(sessionScanAct, &QAction::triggered, [this]() {
        SessionScannerDialog scanner;
        scanner.exec();
    });

    QAction * diagnosticsAction = toolsMenu->addAction(tr("Trouble Code Scanner"));
    connect(diagnosticsAction, &QAction::triggered, [this]() { diagnosticsWindow_.show(); });

    setMenuBar(menuBar);
}

void MainWindow::setupStatusBar()
{
    auto * comboPlatform = new QComboBox;
    comboPlatform->setModel(new PlatformsModel(&LT()->definitions(), this));
    connect(comboPlatform, QOverload<int>::of(&QComboBox::currentIndexChanged), [comboPlatform](int) {
        QVariant var = comboPlatform->currentData(Qt::UserRole);
        if (!var.canConvert<lt::PlatformPtr>())
        {
            return;
        }
        LT()->setPlatform(var.value<lt::PlatformPtr>());
    });

    comboDatalink_ = new QComboBox;
    comboDatalink_->setModel(&linksList_);

    connect(comboDatalink_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        QVariant var = comboDatalink_->currentData(Qt::UserRole);
        if (!var.canConvert<lt::DataLink *>())
        {
            return;
        }

        LT()->setDatalink(var.value<lt::DataLink *>());
    });

    if (LT()->platform())
    {
        comboPlatform->setCurrentText(QString::fromStdString(LT()->platform()->name));
    }
    if (LT()->datalink())
    {
        comboDatalink_->setCurrentText(QString::fromStdString(LT()->datalink()->name()));
    }

    statusBar()->addPermanentWidget(comboPlatform);
    statusBar()->addPermanentWidget(comboDatalink_);
}

void MainWindow::on_buttonDownloadRom_clicked()
{
    /*if (downloadWindow_) {
        delete downloadWindow_;
        downloadWindow_ = nullptr;
    }*/

    DownloadWindow downloadWindow;
    downloadWindow.exec();
}

void MainWindow::newLogClicked()
{
    auto * window = new DataLoggerWindow;
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowModality(Qt::WindowModal);
    window->show();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    if (!checkSave())
    {
        event->ignore();
        return;
    }
    /*canViewer_.close();
    datalinksWindow_.close();*/

    saveSettings();
}

void MainWindow::newProject()
{
    // Wrap in guard
    catchCritical(
        [&]() {
            NewProjectDialog dlg;
            if (dlg.exec() == QDialog::Accepted)
            {
                std::filesystem::path path = dlg.path().toStdString();
                if (dlg.open())
                {
                    if (LT()->openProject(path))
                        addRecent(dlg.path());
                }
                else
                {
                    LT()->createProject(path, path.filename().string());
                    addRecent(dlg.path());
                }
            }
        },
        tr("Error opening project"));
}

void MainWindow::openProject()
{
    QString path = QFileDialog::getExistingDirectory(nullptr, "LibreTuner Project Directory");
    if (path.isNull())
        return;

    if (LT()->openProject(path.toStdString()))
        addRecent(path);
}

void MainWindow::openTune(const lt::TunePtr & tune) { setTune(tune); }

void MainWindow::addRecent(const QString & path)
{
    // Check for duplicates
    if (recentProjects_.contains(path))
    {
        // Move to top of list
        recentProjects_.removeAll(path);
        // Append and don't update menu
        recentProjects_.append(path);
        return;
    }

    recentProjects_.append(path);
    addToRecentMenu(path);
}

void MainWindow::addToRecentMenu(const QString & path)
{
    QAction * action = recentMenu_->addAction(path);
    connect(action, &QAction::triggered, [this, path]() {
        LT()->openProject(path.toStdString());
        addRecent(path);
    });
}

void MainWindow::openCreateTune()
{
    // Check if an unsaved tune is in the workspace first
    if (!checkSave())
        return;

    CreateTuneDialog dlg;
    dlg.exec();
    lt::TunePtr tune = dlg.tune();
    if (!tune)
        return;

    setTune(dlg.tune());
}
