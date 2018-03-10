#include "canlog.h"

void CanLog::addMessage(const CanMessage& message)
{
    std::lock_guard<std::mutex> guard(mutex_);
    beginInsertRows(QModelIndex(), messages_.size(), messages_.size());
    messages_.push_back(message);
    endInsertRows();
}



CanLog::~CanLog()
{
    emit destructing();
}



QVariant CanLog::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            switch(section)
            {
                case 0:
                    return QVariant("ID");
                case 1:
                    return QVariant("Length");
                case 2:
                    return QVariant("Message");
            }
        }
    }
    
    return QVariant();
}



int CanLog::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return size();
}



int CanLog::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 3;
}



QVariant CanLog::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    if (index.row() >= size())
    {
        return QVariant();
    }
    
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }
    
    std::lock_guard<std::mutex> guard(mutex_);
    const CanMessage &message = messages_[index.row()];
    
    switch(index.column())
    {
        case 0:
            return QVariant(QString::number(message.id(), 16));
        case 1:
            return QVariant(message.length());
        case 2:
            return QVariant(QString::fromStdString(message.strMessage()));
    }
    
    
    return QVariant();
}
