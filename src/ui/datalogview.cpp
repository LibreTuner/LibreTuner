#include "datalogview.h"
#include "logger.h"
#include "qcustomplot.h"

#include <QVBoxLayout>


DataLogView::DataLogView(QWidget *parent) : QWidget(parent)
{
    plot_ = new QCustomPlot;
    plot_->legend->setVisible(true);
    plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    auto *layout = new QVBoxLayout;
    layout->addWidget(plot_);
    
    setLayout(layout);
}


void DataLogView::setDatalog(DataLog *log)
{
    if (log != nullptr) {
        connection_ = log->connectUpdate([this] (const DataLog::Data &data, double value, DataLog::TimePoint time) {
            QMetaObject::invokeMethod(this, [=]() {
               append(data, value, time);
            });
        });
    }

    log_ = log;
}

void DataLogView::append(const DataLog::Data &data, double value, DataLog::TimePoint time)
{
    std::size_t graphId;
    auto it = graphs_.find(data.id.id);
    if (it != graphs_.end()) {
        graphId = it->second;
    } else {
        graphId = plot_->graphCount();
        plot_->addGraph();
        plot_->graph(graphId)->setName(QString::fromStdString(data.id.name));
        graphs_.emplace(data.id.id, graphId);
    }
    // Get time since start
    auto duration = time - log_->beginTime();
    
    using milliseconds = std::chrono::milliseconds;
    
    auto diff = std::chrono::duration_cast<milliseconds>(duration).count() / 1000.0;

    plot_->graph(graphId)->addData(diff, value);
    plot_->graph(graphId)->rescaleAxes(true);
    plot_->replot();
}
