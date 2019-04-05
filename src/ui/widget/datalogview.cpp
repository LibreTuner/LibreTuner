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
    
    auto layout = new QHBoxLayout;
    layout->addWidget(plot_);
    setLayout(layout);
}

QCPGraph * DataLogView::getOrCreateGraph(std::size_t pid) noexcept
{
    auto it = graphs_.find(pid);
    if (it == graphs_.end()) {
        QCPGraph *graph = plot_->addGraph();
        graphs_.emplace(pid, graph);
        return graph;
    }
    return it->second;
}

void DataLogView::onAdded(const lt::PidLog& log, const lt::PidLogEntry& entry) noexcept
{
    QCPGraph *graph = getOrCreateGraph(log.pid.code);
    graph->addData(entry.time, entry.value);
}

void DataLogView::setDataLog(lt::DataLogPtr dataLog)
{
    dataLog_ = std::move(dataLog);
    plot_->clearGraphs();
    
    if (!dataLog_) {
        connection_.reset();
        return;
    }
    
    connection_ = dataLog_->onAdd([this](const lt::PidLog &log, const lt::PidLogEntry &entry) {
        onAdded(log, entry);
    });
}
