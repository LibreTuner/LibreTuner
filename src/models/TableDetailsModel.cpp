#include "TableDetailsModel.h"

QModelIndex TableDetailsModel::index(int row, int column, const QModelIndex & parent) const
{
    if (parent.isValid())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TableDetailsModel::parent(const QModelIndex & child) const { return QModelIndex(); }

int TableDetailsModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return 0;

    return 8;
}

int TableDetailsModel::columnCount(const QModelIndex & parent) const { return 2; }

QVariant TableDetailsModel::data(const QModelIndex & index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == 0)
    {
        switch (index.row())
        {
        case 0: // Name
            return tr("Name");
        case 1:
            return tr("Offset");
        case 2:
            return tr("Width");
        case 3:
            return tr("Height");
        case 4:
            return tr("Minimum");
        case 5:
            return tr("Maximum");
        case 6:
            return tr("Unit");
        case 7:
            return tr("Data Type");
        default:
            return QVariant();
        }
    }
    else if (index.column() == 1 && table_ != nullptr)
    {
        switch (index.row())
        {
        case 0: // Name
            return QString::fromStdString(table_->name());
        case 1: // Offset
            return QVariant();
        case 2: // Width
            return QString::number(table_->width());
        case 3: // Height
            return QString::number(table_->height());
        case 4: // Minimum
            return QString::number(table_->minimum());
        case 5: // Maximum
            return QString::number(table_->maximum());
            ;
        case 6: // Unit
            return QVariant();
        case 7: // Data Type
            return QVariant();
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant TableDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (section)
    {
    case 0:
        return tr("Property");
    case 1:
        return tr("Value");
    default:
        return QVariant();
    }
}
void TableDetailsModel::setTable(const lt::Table * table)
{
    if (table_ == table)
        return;
    beginResetModel();
    table_ = table;
    endResetModel();
}
