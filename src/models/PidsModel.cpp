#include "PidsModel.h"
#include <definition/platform.h>

PidsModel::PidsModel(const lt::Platform * platform, QObject * parent) : QAbstractItemModel(parent), platform_{platform}
{
}

void PidsModel::setPlatform(const lt::Platform * platform) {
    beginResetModel();
    platform_ = platform;
    endResetModel();
}

QModelIndex PidsModel::index(int row, int column, const QModelIndex & parent) const
{
    if (platform_ == nullptr)
        return QModelIndex();
    if (parent.isValid())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex PidsModel::parent(const QModelIndex & child) const {
    return QModelIndex(); }

int PidsModel::rowCount(const QModelIndex & parent) const
{
    if (platform_ == nullptr || parent.isValid())
        return 0;

    return platform_->pids.size();
}

int PidsModel::columnCount(const QModelIndex & parent) const {
    return 1;
}

QVariant PidsModel::data(const QModelIndex & index, int role) const {
    if (platform_ == nullptr || !index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= platform_->pids.size())
        return QVariant();

    const lt::Pid & pid = platform_->pids[index.row()];

    if (role == Qt::DisplayRole) {
        return QString::fromStdString(pid.name);
    }

    if (role == Qt::CheckStateRole) {
        return selected_.contains(&pid) ? Qt::Checked : Qt::Unchecked;
    }

    return QVariant();
}

Qt::ItemFlags PidsModel::flags(const QModelIndex & index) const
{
    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PidsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

bool PidsModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (platform_ == nullptr || !index.isValid())
        return false;

    if (index.row() < 0 || index.row() >= platform_->pids.size())
        return false;

    const lt::Pid & pid = platform_->pids[index.row()];

    if (role != Qt::CheckStateRole)
        return false;

    if (value.value<Qt::CheckState>() == Qt::Checked) {
        if (!selected_.contains(&pid)) {
            selected_.append(&pid);
            return true;
        }
    } else {
        selected_.removeAll(&pid);
        return true;
    }

    return false;
}