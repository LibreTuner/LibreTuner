#ifndef CANLOG_H
#define CANLOG_H

#include "caninterface.h"
#include <vector>
#include <mutex>
#include <QAbstractTableModel>

class CanLog : public QAbstractTableModel
{
    Q_OBJECT
public:
    void addMessage(const CanMessage &message);
    
    size_t size() const
    {
        return messages_.size();
    }
    
    const CanMessage &get(size_t index) const
    {
        return messages_[index];
    }
    
    
    int columnCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    int rowCount(const QModelIndex & parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    
    ~CanLog();
private:
    std::vector<CanMessage> messages_;
    mutable std::mutex mutex_;
    
signals:
    void dataChanged(int first, int last);
    void destructing();
};

#endif // CANLOG_H
