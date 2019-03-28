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
    
    void setDataLog(lt::DataLogPtr datalog);
    
private:
    QCustomPlot *plot_;
    QListWidget *pidList_;
    
    lt::DataLogPtr datalog_;
    
    // Map PIDs to graphs
    std::unordered_map<std::size_t, QCPGraph*> graphs_;
};

#endif // DATALOGVIEW_H
