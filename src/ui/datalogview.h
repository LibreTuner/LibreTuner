#ifndef DATALOGVIEW_H
#define DATALOGVIEW_H

#include <QWidget>
#include <QChartView>
#include <QChart>
#include <QLineSeries>

#include <unordered_map>

#include "datalog/datalog.h"

// An interactive graph of datalogs
class DataLogView : public QWidget
{
    Q_OBJECT
public:
    explicit DataLogView(QWidget *parent = nullptr);

    void setDatalog(DataLog *log);

    void append(const DataLog::Data &data, double value);

signals:

public slots:

private:
    QtCharts::QChartView *chartView_;
    QtCharts::QChart *chart_;

    DataLog *log_{nullptr};

    std::unordered_map<std::size_t, QtCharts::QLineSeries*> series_;

    std::shared_ptr<Signal<DataLog::UpdateCall>::ConnectionType> connection_;
};

#endif // DATALOGVIEW_H
