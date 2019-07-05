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

#include "graphwidget.h"
#include "lt/rom/table.h"

#include <QCategory3DAxis>
#include <QChart>
#include <QHBoxLayout>

using namespace QtCharts;

GraphWidget::GraphWidget(QWidget *parent) : QWidget(parent) {
    surface_ = new QtDataVisualization::Q3DSurface;
    container_ = QWidget::createWindowContainer(surface_);
    chart_ = new QChart();
    chart_->legend()->hide();
    chartView_ = new QChartView(chart_, this);
    chartView_->setRenderHint(QPainter::Antialiasing);

    chartView_->setVisible(false);
    container_->setVisible(false);

    auto *hLayout = new QHBoxLayout;
    setLayout(hLayout);
    hLayout->addWidget(container_);
    hLayout->addWidget(chartView_);

    series3d_ = new QtDataVisualization::QSurface3DSeries;
    // I have no fucking clue if Q3DSeries takes ownership of series

    surface_->addSeries(series3d_);
    surface_->setHorizontalAspectRatio(1.0);

    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::green);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(1.0, Qt::red);

    series3d_->setBaseGradient(gr);
    series3d_->setColorStyle(
        QtDataVisualization::Q3DTheme::ColorStyleRangeGradient);
}

GraphWidget::~GraphWidget() { delete container_; }

void GraphWidget::setModel(TableModel *model) {
    if (model_ != nullptr) {
        disconnect(model, &TableModel::modelReset, this, &GraphWidget::refresh);
    }
    model_ = model;
    if (model != nullptr) {
        refresh();

        connect(model, &TableModel::modelReset, this, &GraphWidget::refresh);
    }
}

void GraphWidget::refresh() {
    if (model_ == nullptr) {
        return;
    }
    lt::Table *table = model_->table();
    if (table == nullptr) {
        return;
    }

    if (table->height() > 1) {
        // Two dimensional
        auto *modelProxy =
            new QtDataVisualization::QItemModelSurfaceDataProxy(model_);
        modelProxy->setUseModelCategories(true);
        series3d_->setDrawMode(
            QtDataVisualization::QSurface3DSeries::DrawSurfaceAndWireframe);

        series3d_->setDataProxy(modelProxy);
/*
        if (table->axisX()) {
            surface_->axisX()->setTitle(
                QString::fromStdString(table->axisX()->name()));
            surface_->axisX()->setTitleVisible(true);
        } else {
            surface_->axisX()->setTitleVisible(false);
        }

        if (table->axisY()) {
            surface_->axisZ()->setTitle(
                QString::fromStdString(table->axisY()->name()));
            surface_->axisZ()->setTitleVisible(true);
        } else {
            surface_->axisZ()->setTitleVisible(true);
        }*/

        chartView_->setVisible(false);
        container_->setVisible(true);
    } else if (table->height() == 1 && table->width() > 1) {
        auto *series = new QLineSeries;
        /*if (table->axisX()) {
            for (int x = 0; x < table->width(); ++x) {
                series->append(table->axisX()->label(x),
                               table->get(x, 0)); // Should always be a float
            }
        } else {
            for (int x = 0; x < table->width(); ++x) {
                series->append(x, table->get(x, 0)); // Should always be a float
            }
        }

        chart_->removeAllSeries();
        chart_->addSeries(series);
        chart_->createDefaultAxes();

        if (table->axisX()) {
            chart_->axisX()->setTitleText(
                QString::fromStdString(table->axisX()->name()));
        }

        chartView_->setVisible(true);
        container_->setVisible(false);*/
    } else {
        chartView_->setVisible(false);
        container_->setVisible(false);
    }
}
