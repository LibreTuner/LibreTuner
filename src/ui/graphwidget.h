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

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <Q3DSurface>
#include <QItemModelSurfaceDataProxy>
#include <QSurface3DSeries>
#include <QWidget>
#include <QtCharts>

#include <memory>

class Table;
typedef std::shared_ptr<Table> TablePtr;

/**
 * @todo write docs
 */
class GraphWidget : public QWidget {
public:
    explicit GraphWidget(QWidget *parent = nullptr);

public slots:
    void tableChanged(const TablePtr &table);

private:
    TablePtr table_;
    QtDataVisualization::Q3DSurface *surface_;
    QWidget *container_;
    QtCharts::QChart *chart_;
    QtCharts::QChartView *chartView_;

    QtDataVisualization::QSurface3DSeries series3d_;
};

#endif // GRAPHWIDGET_H
