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

#ifndef DATALOGGERWINDOW_H
#define DATALOGGERWINDOW_H

#include <QPushButton>
#include <QTreeWidget>
#include <QWidget>

#include <memory>
#include <unordered_map>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "lt/datalog/datalog.h"

namespace lt {
class DataLogger;
using DataLoggerPtr = std::unique_ptr<DataLogger>;
}

class QListWidget;
class QTreeWidgetItem;
class QListWidgetItem;
class DataLogView;
class DataLogLiveView;

class DataLoggerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DataLoggerWindow(QWidget *parent = nullptr);

    ~DataLoggerWindow() override;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    
    // Simulate PIDs
    void simulate();

    void resetLog();

    void waitForStop();

signals:

public slots:
    /* Callback for the start/stop button */
    void toggleLogger();
    void saveLog();

private:
    lt::DataLogPtr log_;
    lt::DataLoggerPtr logger_;

    QListWidget *pidList_;
    QPushButton *buttonLog_;
    DataLogView *dataLogView_;
    DataLogLiveView *dataLogLiveView_;

    std::vector<QListWidgetItem*> pidItems_;

    std::condition_variable cv_;
    std::mutex mutex_;

    void reset();
};

#endif // DATALOGGERWINDOW_H
