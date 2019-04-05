#include "datalogliveview.h"

#include <QVBoxLayout>
#include <QStringList>

DataLogLiveView::DataLogLiveView(QWidget* parent) : QTreeWidget(parent)
{
    setColumnCount(2);
    
    QStringList headerLabels;
    headerLabels << tr("Name") << tr("Last Value");
    setHeaderLabels(headerLabels);
}


void DataLogLiveView::setDataLog(lt::DataLogPtr dataLog) noexcept
{
    dataLog_ = std::move(dataLog);
    clear();
    pids_.clear();
    
    if (!dataLog_) {
        connection_.reset();
        return;
    }
    
    connection_ = dataLog_->onAdd([this](const lt::PidLog &log, const lt::PidLogEntry &entry) {
        onAdded(log, entry);
    });
}


void DataLogLiveView::onAdded(const lt::PidLog& log, const lt::PidLogEntry& entry) noexcept
{
    auto it = pids_.find(log.pid.code);
    
    QTreeWidgetItem *item;
    if (it == pids_.end()) {
        item = new QTreeWidgetItem;
        item->setText(0, QString::fromStdString(log.pid.name));
        addTopLevelItem(item);
        pids_.emplace(log.pid.code, item);
    } else {
        item = it->second;
    }
    
    item->setData(1, Qt::DisplayRole, entry.value);
}
