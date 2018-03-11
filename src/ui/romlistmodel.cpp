#include "romlistmodel.h"

QVariant RomListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    
    if (role == Qt::DisplayRole)
    {
        return QVariant("Item " + QString::number(index.row()));
    }
    if (role == Qt::DecorationRole)
    {
        return QVariant();
    }
    
    return QVariant();
}

int RomListModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

int RomListModel::rowCount(const QModelIndex& parent) const
{
    return 20;
}
