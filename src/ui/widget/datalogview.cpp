#include "datalogview.h"

#include "../qcustomplot.h"

#include <QHBoxLayout>
#include <QListWidget>

DataLogView::DataLogView(QWidget* parent) : QWidget(parent)
{
    plot_ = new QCustomPlot;
    
    plot_->legend->setVisible(true);
    plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    plot_->xAxis->setLabel("Elapsed time (seconds)");
    
    pidList_ = new QListWidget;
    
    auto layout = new QHBoxLayout;
    layout->addWidget(pidList_);
    layout->addWidget(plot_);
    setLayout(layout);
}

void DataLogView::setDataLog(lt::DataLogPtr datalog)
{
    datalog_ = std::move(datalog);
}
