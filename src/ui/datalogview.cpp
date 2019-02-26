#include "datalogview.h"


DataLogView::DataLogView(QWidget *parent) : QWidget(parent)
{
    chart_ = new QtCharts::QChart;
    chartView_ = new QtCharts::QChartView(chart_);


}


Q_DECLARE_METATYPE(DataLog::Data)


void DataLogView::setDatalog(DataLog *log)
{
    if (log != nullptr) {
        log->connectUpdate([this] (const DataLog::Data &data, double value) {
            append(data, value);
        });
    }

    log_ = log;
}

void DataLogView::append(const DataLog::Data &data, double value)
{
    auto it = series_.find(data.id.id);

    QtCharts::QLineSeries *series;

    if (it != series_.end()) {
        series = it->second;
    } else {
        series = new QtCharts::QLineSeries;
        chart_->addSeries(series);
    }
}
