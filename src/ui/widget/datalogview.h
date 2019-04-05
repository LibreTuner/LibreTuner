#ifndef DATALOGVIEW_H
#define DATALOGVIEW_H

#include <QWidget>
#include <unordered_map>

#include "lt/datalog/datalog.h"

class QCPGraph;
class QCustomPlot;
class QListWidget;

/**
 * @todo write docs
 */
class DataLogView : public QWidget
{
public:
    explicit DataLogView(QWidget *parent = nullptr);
    
    void setDataLog(lt::DataLogPtr dataLog);
    
private:
    void onAdded(const lt::PidLog &log, const lt::PidLogEntry &entry) noexcept;
    
    QCPGraph *getOrCreateGraph(std::size_t pid) noexcept;
    
    QCustomPlot *plot_;
    
    lt::DataLogPtr dataLog_;
    lt::DataLog::AddConnectionPtr connection_;
    
    // Map PIDs to graphs
    std::unordered_map<std::size_t, QCPGraph*> graphs_;
};

#endif // DATALOGVIEW_H
