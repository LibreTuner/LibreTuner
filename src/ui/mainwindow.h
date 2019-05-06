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

#include <QComboBox>
#include <QLayout>
#include <QMainWindow>
#include <QPointer>

#include <filesystem>

#include "database/links.h"
#include "models/tablemodel.h"
#include "lt/rom/rom.h"
#include "datalinkswidget.h"

class QListView;

class EditorWidget;
class TablesWidget;
class TuneData;
class SidebarWidget;
class GraphWidget;
class DefinitionsWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    // If newPath is true OR the current tune was not loaded from a file, asks
    // the user for a path.
    void saveTune(bool newPath = false);

public slots :
    void setTable(const lt::ModelTable *modTable);

private slots:
    void on_buttonDownloadRom_clicked();
    void newLogClicked();
    void closeEvent(QCloseEvent *event) override;
    
signals:
    void tableChanged(lt::Table *table);

    void tuneChanged(const lt::Tune *tune);

private:
    bool checkSave();

    QComboBox *comboLogVehicles_;
    QComboBox *comboDatalink_;
    QListView *listLogs_;
    SidebarWidget *sidebar_;

    TablesWidget *tables_;
    EditorWidget *editor_;
    GraphWidget *graph_;
    
    QAction *flashCurrentAction_;
    QAction *saveCurrentAction_;

    // Docks
    QDockWidget *logDock_;
    QDockWidget *overviewDock_;
    QDockWidget *loggingDock_;
    QDockWidget *diagnosticsDock_;
    QDockWidget *sidebarDock_;
    QDockWidget *tablesDock_;
    QDockWidget *editorDock_;
    QDockWidget *graphDock_;
    

    void setupMenu();
    void setupStatusBar();
    
    void hideAllDocks();
    void restoreDocks();
    
    void saveSettings();
    void loadSettings();

    void setTune(const lt::TunePtr &tune, const std::filesystem::path &path = std::filesystem::path());

    QDockWidget *createOverviewDock();
    QDockWidget *createLoggingDock();
    QDockWidget *createDiagnosticsDock();
    QDockWidget *createLogDock();
    QDockWidget *createSidebarDock();
    QDockWidget *createTablesDock();
    QDockWidget *createEditorDock();
    QDockWidget *createGraphDock();
    
    std::vector<QDockWidget*> docks_;

    std::filesystem::path tunePath_;

    lt::TunePtr tune_;

    LinksListModel linksList_;
    TableModel tableModel_;
    
    DatalinksWidget datalinksWindow_;

    QPointer<DefinitionsWindow> definitionsWindow_;
};

#endif // MAINWINDOW_H
