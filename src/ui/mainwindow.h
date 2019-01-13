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
#include "datalinkswidget.h"

#include <QComboBox>
#include <QLayout>
#include <QMainWindow>

class QListView;

class EditorWidget;
class TablesWidget;
class TuneData;
class SidebarWidget;
class GraphWidget;
class Table;
class Tune;

class MainWindow : public QMainWindow { // public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

private slots:
    void on_buttonDownloadRom_clicked();
    void newLogClicked();
    void closeEvent(QCloseEvent *event) override;
    
signals:
    void tableChanged(Table *table);

private:
    CanViewer canViewer_;
    DownloadWindow *downloadWindow_ = nullptr;
    DatalinksWidget datalinksWindow_;

    QComboBox *comboLogVehicles_;
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
    
    bool changeSelected(Tune *tune);
    
    // Returns true if it is safe to discard the tune data
    bool checkSaveSelected();

    void setupMenu();
    void setupStatusBar();
    
    void hideAllDocks();
    void restoreDocks();
    
    void saveSettings();
    void loadSettings();

    QDockWidget *createOverviewDock();
    QDockWidget *createLoggingDock();
    QDockWidget *createDiagnosticsDock();
    QDockWidget *createLogDock();
    QDockWidget *createSidebarDock();
    QDockWidget *createTablesDock();
    QDockWidget *createEditorDock();
    QDockWidget *createGraphDock();
    
    std::vector<QDockWidget*> docks_;
    
    std::shared_ptr<TuneData> selectedTune_;
};

#endif // MAINWINDOW_H
