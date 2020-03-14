#include "CategorizedTablesModel.h"

CategorizedTablesModel::CategorizedTablesModel(const lt::Model * model, QObject * parent) : QAbstractItemModel(parent)
{
    setDefinition(model);
}

QModelIndex CategorizedTablesModel::index(int row, int column, const QModelIndex & parent) const
{
    if (model_ == nullptr)
        return QModelIndex();

    if (!parent.isValid())
    {
        // Category index
        if (row < 0 || row >= categories_.size())
            return QModelIndex();
        return createIndex(row, column, static_cast<quintptr>(0));
    }

    // Table index
    const TableCategory & tc = categories_[parent.row()];
    if (row < 0 || row >= tc.tables.size())
        return QModelIndex();
    return createIndex(row, column, parent.row() + 1);
}

QModelIndex CategorizedTablesModel::parent(const QModelIndex & child) const
{
    if (model_ == nullptr)
        return QModelIndex();

    if (!child.isValid() || child.internalId() == 0)
        return QModelIndex();

    return createIndex(child.internalId() - 1, 0, static_cast<quintptr>(0));
}

int CategorizedTablesModel::rowCount(const QModelIndex & parent) const
{
    if (model_ == nullptr)
        return 0;

    if (!parent.isValid())
        return categories_.size();

    // Check if parent is a table
    if (parent.internalId() != 0)
        return 0;

    if (parent.row() < 0 || parent.row() >= categories_.size())
        return 0;

    return categories_[parent.row()].tables.size();
}

int CategorizedTablesModel::columnCount(const QModelIndex & parent) const
{
    if (model_ == nullptr)
        return 0;

    return 1;
}

QVariant CategorizedTablesModel::data(const QModelIndex & index, int role) const
{
    if (model_ == nullptr || !index.isValid())
        return QVariant();

    if (index.internalId() != 0)
    {
        // This is a table
        const lt::TableDefinition * td = categories_[index.internalId() - 1].tables[index.row()];

        switch (role)
        {
        case Qt::DisplayRole:
            return QString::fromStdString(td->name);
        case Qt::UserRole:
            return QVariant::fromValue(td);
        default:
            return QVariant();
        }
    }

    const TableCategory & tc = categories_[index.row()];
    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(tc.name);
    default:
        return QVariant();
    }
}

void CategorizedTablesModel::setDefinition(const lt::Model * model)
{
    if (model_ == model)
        return;
    model_ = model;

    beginResetModel();

    // Rebuild tree
    categories_.clear();

    if (model_ == nullptr)
    {
        endResetModel();
        return;
    }

    for (const auto & [id, table] : model->tables)
    {
        const std::string & category = table.category;
        if (auto it = std::find_if(categories_.begin(), categories_.end(),
                                   [&](const TableCategory & tc) { return tc.name == category; });
            it != categories_.end())
        {
            it->tables.emplace_back(&table);
        }
        else
        {
            TableCategory tc{category};
            tc.tables.emplace_back(&table);
            categories_.emplace_back(std::move(tc));
        }
    }
    endResetModel();
}

QVariant CategorizedTablesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return tr("Name");
    }
    return QVariant();
}
