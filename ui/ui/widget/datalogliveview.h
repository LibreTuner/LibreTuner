#ifndef DATALOGLIVEVIEW_H
#define DATALOGLIVEVIEW_H

#include <QTreeWidget>
#include <QWidget>

#include <memory>
#include <unordered_map>

#include "lt/datalog/datalog.h"

class DataLogLiveView : public QTreeWidget
{
public:
    explicit DataLogLiveView(QWidget * parent = nullptr);

    void setDataLog(lt::DataLogPtr dataLog) noexcept;

private:
    lt::DataLogPtr dataLog_;

    lt::DataLog::AddConnectionPtr connection_;

    std::unordered_map<std::size_t, QTreeWidgetItem *> pids_;

    void onAdded(const lt::PidLog & log,
                 const lt::PidLogEntry & entry) noexcept;
};

#endif // DATALOGLIVEVIEW_H
