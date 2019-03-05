#include "datalinkslistmodel.h"
#include "datalink/linkdatabase.h"
#include "datalink/datalink.h"

Q_DECLARE_METATYPE(datalink::Link*)

QVariant DataLinksListModel::data(const QModelIndex& index, int role) const
{
    if (links_ == nullptr) {
        return QVariant();
    }
    if (index.column() != 0) {
        return QVariant();
    }
    
    if (index.row() == 0 && links_->count() == 0) {
        if (role == Qt::DisplayRole) {
            return tr("No datalinks");
        }
        return QVariant();
    }
    if (index.row() < 0 || index.row() >= links_->count()) {
        return QVariant();
    }

    
    datalink::Link *link = links_->get(index.row());
    
    if (role == Qt::DisplayRole) {
        return QString::fromStdString(link->name());
    } else if (role == Qt::UserRole) {
        return QVariant::fromValue(link);
    }
    return QVariant();
}

int DataLinksListModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

int DataLinksListModel::rowCount(const QModelIndex& parent) const
{
    return std::max<int>(1, links_ != nullptr ? links_->count() : 0);
}

QModelIndex DataLinksListModel::parent(const QModelIndex& child) const
{
    return QModelIndex();
}
