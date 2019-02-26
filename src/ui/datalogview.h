#ifndef DATALOGVIEW_H
#define DATALOGVIEW_H

#include <QWidget>

#include "datalog/datalog.h"

#include <unordered_map>

class QCustomPlot;

// An interactive graph for analyzing datalogs
class DataLogView : public QWidget
{
    Q_OBJECT
public:
    explicit DataLogView(QWidget *parent = nullptr);

    void setDatalog(DataLog *log);

    void append(const DataLog::Data &data, double value, DataLog::TimePoint time);

signals:

public slots:

private:
    QCustomPlot *plot_;
    
    DataLog *log_{nullptr};

    // Map PIDs to graph ids
    std::unordered_map<std::size_t, std::size_t> graphs_;

    std::shared_ptr<Signal<DataLog::UpdateCall>::ConnectionType> connection_;
};

#endif // DATALOGVIEW_H
