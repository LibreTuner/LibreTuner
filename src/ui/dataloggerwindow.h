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

#include "styledwindow.h"
#include "datalog/datalog.h"

class DataLogger;

class QListWidget;
class QTreeWidgetItem;
class QListWidgetItem;

class PlatformLink;
using PlatformLinkPtr = std::shared_ptr<PlatformLink>;


class DataLoggerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DataLoggerWindow(QWidget *parent = nullptr);

    ~DataLoggerWindow() override;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

signals:

public slots:
    /* Callback for the start/stop button */
    void buttonClicked();



private:
    DataLog log_;
    std::unique_ptr<DataLogger> logger_;
    definition::MainPtr definition_;

    QListWidget *pidList_;
    QTreeWidget *logOutput_;
    QPushButton *buttonLog_;
    
    std::shared_ptr<Signal<DataLog::UpdateCall>::ConnectionType> connection_;
    
    void onLogEntry(const DataLog::Data &data, double value);
    
    std::unordered_map<uint32_t, QTreeWidgetItem*> outputItems_;
    std::vector<QListWidgetItem*> pidItems_;


    void reset();
};

#endif // DATALOGGERWINDOW_H
