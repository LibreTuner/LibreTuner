#include "datalogview.h"
#include "logger.h"
#include "qcustomplot.h"

#include <QVBoxLayout>

DataLogView::DataLogView(QWidget *parent) : QWidget(parent) {
    plot_ = new QCustomPlot;
    plot_->legend->setVisible(true);
    plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                           QCP::iSelectPlottables);

    plot_->xAxis->setLabel("Elapsed time (seconds)");

    auto *layout = new QVBoxLayout;
    layout->addWidget(plot_);

    setLayout(layout);
}

void DataLogView::setDatalog(DataLog *log) {
    connection_.reset();
    plot_->clearGraphs();
    if (log != nullptr) {
        connection_ =
            log->connectUpdate([this](const DataLog::Data &data, double value,
                                      DataLog::TimePoint time) {
                QMetaObject::invokeMethod(this,
                                          [=]() { append(data, value, time); });
            });
    }

    log_ = log;
}

void DataLogView::append(const DataLog::Data &data, double value,
                         DataLog::TimePoint time) {
    static QColor plotColors[] = {
        Qt::red,       Qt::green,    Qt::blue,        Qt::magenta,
        Qt::yellow,    Qt::gray,     Qt::cyan,        Qt::darkRed,
        Qt::darkGreen, Qt::darkBlue, Qt::darkMagenta, Qt::darkYellow,
        Qt::darkGray,  Qt::darkCyan, Qt::lightGray,
    };

    int graphId;
    if (graphs_.size() <= data.id.id) {
        graphs_.resize(data.id.id + 1, -1);
    }
    auto it = graphs_[data.id.id];
    if (it != -1) {
        graphId = it;
    } else {
        graphId = plot_->graphCount();
        plot_->addGraph();
        plot_->graph(graphId)->setName(QString::fromStdString(data.id.name));

        // Set next color
        plot_->graph(graphId)->setPen(
            QPen(plotColors[graphId % sizeof(plotColors)]));

        graphs_[data.id.id] = graphId;
    }
    // Get time since start
    auto duration = time - log_->beginTime();

    using milliseconds = std::chrono::milliseconds;

    auto diff =
        std::chrono::duration_cast<milliseconds>(duration).count() / 1000.0;

    plot_->graph(graphId)->addData(diff, value);
    plot_->xAxis->setRange(diff, 8, Qt::AlignRight);
    plot_->replot();
}
