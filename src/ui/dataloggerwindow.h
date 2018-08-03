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

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include "vehicle.h"

#include <memory>

class QListWidget;

class VehicleLink;
using VehicleLinkPtr = std::shared_ptr<VehicleLink>;

class DataLogger;
using DataLoggerPtr = std::shared_ptr<DataLogger>;

class DataLog;
using DataLogPtr = std::shared_ptr<DataLog>;

class DataLoggerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DataLoggerWindow(const DataLogPtr &log, const DataLoggerPtr &logger, DefinitionPtr definition, QWidget *parent = nullptr);
    virtual ~DataLoggerWindow() = default;

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

signals:

public slots:
    /* Callback for the start/stop button */
    void buttonClicked();

private:
    DataLogPtr log_;
    DataLoggerPtr logger_;
    DefinitionPtr definition_;

    QListWidget *pidList_;
    QTreeWidget *logOutput_;
    QPushButton *buttonLog_;
};

#endif // DATALOGGERWINDOW_H
