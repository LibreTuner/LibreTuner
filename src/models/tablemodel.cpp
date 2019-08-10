#include "tablemodel.h"

#include <QColor>

#include <cmath>

void TableModel::setTable(lt::Table * table) noexcept
{
    beginResetModel();
    table_ = table;
    endResetModel();
}

int TableModel::rowCount(const QModelIndex & parent) const
{
    if (table_ == nullptr || parent.isValid())
        return 0;

    return table_->height();
}

int TableModel::columnCount(const QModelIndex & parent) const
{
    if (table_ == nullptr || parent.isValid())
        return 0;

    return table_->width();
}

QVariant TableModel::data(const QModelIndex & index, int role) const
{
    if (table_ == nullptr || !index.isValid())
        return QVariant();

    if (role != Qt::UserRole && role != Qt::DisplayRole && role != Qt::BackgroundColorRole &&
        role != Qt::ForegroundRole)
        return QVariant();

    if (index.row() < 0 || index.row() >= table_->height() || index.column() < 0 || index.column() >= table_->width())
        return QVariant();

    if (role == Qt::DisplayRole)
        return table_->get(index.row(), index.column());

    if (role == Qt::ForegroundRole)
    {
        if (table_->isSingle())
            return QVariant();

        return QColor(0, 0, 0);
    }

    if (role == Qt::BackgroundColorRole)
    {
        if (table_->isSingle())
            return QVariant();

        double diff = table_->maximum() - table_->minimum();
        if (diff == 0.0)
            return QColor::fromHsvF((1.0 / 3.0), 1.0, 1.0);
        double ratio = static_cast<double>(table_->get(index.row(), index.column()) - table_->minimum()) / diff;
        ratio = std::clamp(ratio, 0.0, 1.0);
        return QColor::fromHsvF((1.0 - ratio) * (1.0 / 3.0), 1.0, 1.0);
    }

    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (table_ == nullptr || role != Qt::DisplayRole)
        return QVariant();

    lt::AxisPtr axis;
    if (orientation == Qt::Horizontal)
        axis = table_->xAxis();
    else if (orientation == Qt::Vertical)
        axis = table_->yAxis();

    if (!axis)
        return QVariant();

    return QString::number(std::floor(axis->index(section) * 100.0) / 100.0);
}

bool TableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (table_ == nullptr || !index.isValid())
    {
        return false;
    }

    if (role != Qt::EditRole)
    {
        return false;
    }

    if (index.row() < 0 || index.row() >= table_->height() || index.column() < 0 || index.column() >= table_->width())
    {
        return false;
    }

    bool ok;
    double val = value.toDouble(&ok);
    if (!ok)
    {
        return false;
    }

    table_->set(index.row(), index.column(), val);
    return true;
}

Qt::ItemFlags TableModel::flags(const QModelIndex & index) const
{
    if (index.isValid())
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    }
    return Qt::NoItemFlags;
}
