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

#include "GraphWidget.h"

#include <QCategory3DAxis>
#include <QChart>
#include <QHBoxLayout>

using namespace QtCharts;

GraphWidget::GraphWidget(QWidget * parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 3D
    surface_ = new QtDataVisualization::Q3DSurface;
    container_ = QWidget::createWindowContainer(surface_);

    auto * theme = new QtDataVisualization::Q3DTheme(QtDataVisualization::Q3DTheme::ThemePrimaryColors);
    surface_->addTheme(theme);
    surface_->setActiveTheme(theme);

    series3d_ = new QtDataVisualization::QSurface3DSeries;
    // I have no fucking clue if Q3DSeries takes ownership of series

    modelProxy_ = new QtDataVisualization::QItemModelSurfaceDataProxy;
    modelProxy_->setUseModelCategories(true);
    series3d_->setDrawMode(QtDataVisualization::QSurface3DSeries::DrawSurfaceAndWireframe);
    series3d_->setDataProxy(modelProxy_);

    surface_->addSeries(series3d_);
    surface_->setHorizontalAspectRatio(1.0);
    surface_->axisZ()->setReversed(true);

    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::green);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(1.0, Qt::red);

    series3d_->setBaseGradient(gr);
    series3d_->setColorStyle(QtDataVisualization::Q3DTheme::ColorStyleRangeGradient);

    // 2D
    chart_ = new QChart();
    chart_->legend()->hide();
    chartView_ = new QChartView(chart_, this);
    chartView_->setRenderHint(QPainter::Antialiasing);

    chartView_->setVisible(false);
    // container_->setVisible(false);

    auto * hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);
    hLayout->addWidget(container_);
    hLayout->addWidget(chartView_);
}

GraphWidget::~GraphWidget()
{ /*delete container_;*/
}

void GraphWidget::setModel(TableModel * model)
{
    if (model_ != nullptr)
        disconnect(model, &TableModel::modelReset, this, &GraphWidget::refresh);

    model_ = model;
    if (model != nullptr)
    {
        connect(model, &TableModel::modelReset, this, &GraphWidget::refresh);
    }
    refresh();
}

void GraphWidget::refresh()
{
    if (model_ == nullptr)
    {
        modelProxy_->setItemModel(nullptr);
        chart_->removeAllSeries();
        chartView_->setVisible(false);
        container_->setVisible(false);
        return;
    }

    const lt::Table & table = model_->table();

    if (!table.isOneDimensional())
    {
        // Two dimensional
        modelProxy_->setItemModel(model_);

        if (table.xAxis())
        {
            surface_->axisX()->setTitle(QString::fromStdString(table.xAxis()->name()));
            surface_->axisX()->setTitleVisible(true);
        }
        else
            surface_->axisX()->setTitleVisible(false);

        if (table.yAxis())
        {
            surface_->axisZ()->setTitle(QString::fromStdString(table.yAxis()->name()));
            surface_->axisZ()->setTitleVisible(true);
        }
        else
            surface_->axisZ()->setTitleVisible(true);

        chartView_->setVisible(false);
        // container_->setVisible(true);
        container_->show();
    }
    else if (table.height() == 1 && table.width() > 1)
    {
        auto * series = new QLineSeries;
        if (table.xAxis())
        {
            for (int x = 0; x < table.width(); ++x)
            {
                double index = 0;
                if (x < table.xAxis()->size())
                    index = table.xAxis()->index(x);
                series->append(index, table.get(0, x)); // Should always be a float
            }
        }
        else
        {
            for (int x = 0; x < table.width(); ++x)
            {
                series->append(x, table.get(0, x)); // Should always be a float
            }
        }

        chart_->removeAllSeries();
        chart_->addSeries(series);
        chart_->createDefaultAxes();

        if (table.xAxis())
        {
            chart_->axisX()->setTitleText(QString::fromStdString(table.xAxis()->name()));
        }

        chartView_->setVisible(true);
        container_->setVisible(false);
    }
    else
    {
        chartView_->setVisible(false);
        container_->setVisible(false);
    }
}
