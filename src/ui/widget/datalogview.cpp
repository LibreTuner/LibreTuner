#include "datalogview.h"

#include "../qcustomplot.h"

#include <QCheckBox>
#include <QListWidget>
#include <QVBoxLayout>

DataLogView::DataLogView(QWidget * parent) : QWidget(parent)
{
    plot_ = new QCustomPlot;

    plot_->legend->setVisible(true);
    plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                           QCP::iSelectPlottables);
    plot_->xAxis->setLabel("Elapsed time (seconds)");

    // Set maximum range
    connect(plot_->xAxis, qOverload<const QCPRange &>(&QCPAxis::rangeChanged),
            this, [this](const QCPRange & newRange) {
                if (dataLog_)
                {
                    plot_->xAxis->setRange(
                        newRange.bounded(0, dataLog_->maxTime() / 1000.0));
                }
            });

    connect(plot_->yAxis, qOverload<const QCPRange &>(&QCPAxis::rangeChanged),
            this, [this](const QCPRange & newRange) {
                if (dataLog_)
                {
                    plot_->yAxis->setRange(
                        newRange.bounded(dataLog_->minValue() - 5.0,
                                         dataLog_->maxValue() + 5.0));
                }
            });

    checkLive_ = new QCheckBox(tr("Update range with live data"));
    checkLive_->setCheckState(Qt::Checked);

    auto layout = new QVBoxLayout;
    layout->addWidget(plot_);
    layout->addWidget(checkLive_);
    setLayout(layout);
}

QCPGraph * DataLogView::getOrCreateGraph(const lt::Pid & pid) noexcept
{
    static const QColor plotColors[] = {
        Qt::red,       Qt::green,    Qt::blue,        Qt::magenta,
        Qt::yellow,    Qt::gray,     Qt::cyan,        Qt::darkRed,
        Qt::darkGreen, Qt::darkBlue, Qt::darkMagenta, Qt::darkYellow,
        Qt::darkGray,  Qt::darkCyan, Qt::lightGray,
    };

    auto it = graphs_.find(pid.code);
    if (it == graphs_.end())
    {
        QCPGraph * graph = plot_->addGraph();
        graph->setName(QString::fromStdString(pid.name));
        graph->setPen(QPen(plotColors[graphs_.size() % sizeof(plotColors)]));
        graphs_.emplace(pid.code, graph);
        return graph;
    }
    return it->second;
}

void DataLogView::onAdded(const lt::PidLog & log,
                          const lt::PidLogEntry & entry) noexcept
{
    QMetaObject::invokeMethod(
        this,
        [this, log, entry] {
            QCPGraph * graph = getOrCreateGraph(log.pid);
            graph->addData(static_cast<double>(entry.time) / 1000.0,
                           entry.value);

            if (checkLive_->isChecked())
            {
                plot_->xAxis->setRange(dataLog_->maxTime() / 1000.0, 8,
                                       Qt::AlignRight);
            }

            plot_->replot(QCustomPlot::rpQueuedReplot);
        },
        Qt::QueuedConnection);
}

void DataLogView::setDataLog(lt::DataLogPtr dataLog)
{
    dataLog_ = std::move(dataLog);
    graphs_.clear();
    plot_->clearGraphs();

    if (!dataLog_)
    {
        connection_.reset();
        return;
    }

    connection_ = dataLog_->onAdd(
        [this](const lt::PidLog & log, const lt::PidLogEntry & entry) {
            onAdded(log, entry);
        });
}
