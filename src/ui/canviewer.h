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

#ifndef CANVIEWER_H
#define CANVIEWER_H

#include <QWidget>

namespace Ui {
class CanViewer;
}

class CanHandler;
class QAbstractItemModel;

class CanViewer : public QWidget
{
    Q_OBJECT

public:
    explicit CanViewer(QWidget *parent = 0);
    ~CanViewer();

private:
    Ui::CanViewer *ui;
    QAbstractItemModel *logModel_ = nullptr;
    
public slots:
    void rowsInserted(const QModelIndex &parent, int first, int last);
};

#endif // CANVIEWER_H
