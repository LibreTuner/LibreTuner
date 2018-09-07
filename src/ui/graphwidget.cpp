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
#include "table.h"

#include <QChart>
#include <QHBoxLayout>

using namespace QtCharts;

GraphWidget::GraphWidget(QWidget *parent) : QWidget(parent) {
    surface_ = new QtDataVisualization::Q3DSurface;
    container_ = QWidget::createWindowContainer(surface_, this);
    chart_ = new QChart();
    chart_->legend()->hide();
    chartView_ = new QChartView(chart_, this);
    chartView_->setRenderHint(QPainter::Antialiasing);

    chartView_->setVisible(false);
    container_->setVisible(false);

    auto *hLayout = new QHBoxLayout(this);
    hLayout->addWidget(container_);
    hLayout->addWidget(chartView_);

    series3d_.setDrawMode(
        QtDataVisualization::QSurface3DSeries::DrawSurfaceAndWireframe);
    surface_->addSeries(&series3d_);
    surface_->setHorizontalAspectRatio(1.0);

    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::green);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(1.0, Qt::red);

    series3d_.setBaseGradient(gr);
    series3d_.setColorStyle(
        QtDataVisualization::Q3DTheme::ColorStyleRangeGradient);
}

#include <iostream>

void GraphWidget::tableChanged(const TablePtr &table) {
    table_ = table;
    if (table == nullptr) {
        return;
    }

    if (table->type() == TABLE_2D) {
        auto *modelProxy =
            new QtDataVisualization::QItemModelSurfaceDataProxy(table.get());
        modelProxy->setUseModelCategories(true);
        // modelProxy->setAutoColumnCategories(false);

        series3d_.setDataProxy(modelProxy);
        if (table->definition()->axisX()) {
            surface_->axisX()->setTitle(
                QString::fromStdString(table->definition()->axisX()->label()));
            surface_->axisX()->setTitleVisible(true);
        } else {
            surface_->axisX()->setTitleVisible(false);
        }

        if (table->definition()->axisY()) {
            surface_->axisZ()->setTitle(
                QString::fromStdString(table->definition()->axisY()->label()));
            surface_->axisZ()->setTitleVisible(true);
        } else {
            surface_->axisZ()->setTitleVisible(true);
        }

        chartView_->setVisible(false);
        container_->setVisible(true);
    } else if (table->type() == TABLE_1D && table->definition()->axisX()) {
        auto *series = new QLineSeries;
        for (int x = 0; x < table->definition()->sizeX(); ++x) {
            series->append(table->definition()->axisX()->label(x),
                           table->data(table->index(0, x))
                               .toFloat()); // Should always be a float
        }

        chart_->removeAllSeries();
        chart_->addSeries(series);
        chart_->createDefaultAxes();

        chart_->createDefaultAxes();
        chart_->axisX()->setTitleText(
            QString::fromStdString(table->definition()->axisX()->label()));

        chartView_->setVisible(true);
        container_->setVisible(false);
    }
}
