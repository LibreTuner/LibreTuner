#include "definitions.h"
#include "logger.h"
#include <fstream>

#include <lt/definition/platform.h>

QModelIndex PlatformsModel::index(int row, int column,
                                  const QModelIndex & parent) const
{
    if (!parent.isValid())
    {
        // Creating platform node
        return createIndex(row, column);
    }

    // Creating model node
    return createIndex(row, column, parent.row() + 1);
}

QModelIndex PlatformsModel::parent(const QModelIndex & child) const
{
    if (!child.isValid())
        return QModelIndex();

    if (child.internalId() == 0)
    {
        // Platform node
        return QModelIndex();
    }
    else
    {
        // Model node
        return createIndex(child.internalId() - 1, 0);
    }
}

int PlatformsModel::rowCount(const QModelIndex & parent) const
{
    if (platforms_ == nullptr)
        return 0;
    if (!parent.isValid())
        return platforms_->size();

    if (parent.internalId() == 0)
    {
        // Platform node
        int row = parent.row();
        if (row < 0 || row >= platforms_->size())
            return 0;
        return platforms_->at(parent.row())->models.size();
    }
    return 0;
}

int PlatformsModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QVariant PlatformsModel::data(const QModelIndex & index, int role) const
{
    if (platforms_ == nullptr)
        return 0;
    if (!index.isValid() || index.column() != 0 ||
        (role != Qt::DisplayRole && role != Qt::UserRole))
    {
        return QVariant();
    }

    if (index.internalId() == 0)
    {
        // Platform
        int platformIndex = index.row();
        if (platformIndex >= platforms_->size())
        {
            // platformIndex will never be < 0 due to constraints in views
            return QVariant();
        }
        const lt::PlatformPtr & platform = platforms_->at(platformIndex);
        if (role == Qt::DisplayRole)
        {
            return QString::fromStdString(platform->name);
        }
        else if (role == Qt::UserRole)
        {
            return QVariant::fromValue(platform);
        }
        return QVariant();
    }

    // Model
    int platformIndex = index.internalId() - 1;
    if (platformIndex >= platforms_->size())
    {
        // platformIndex will never be < 0 due to constraints in views
        return QVariant();
    }
    const lt::PlatformPtr & platform = platforms_->at(platformIndex);

    int modelIndex = index.row();

    if (modelIndex < platform->models.size())
    {
        const lt::ModelPtr & model = platform->models[modelIndex];
        if (role == Qt::DisplayRole)
        {
            return QString::fromStdString(model->name);
        }
        else if (role == Qt::UserRole)
        {
            return QVariant::fromValue(model);
        }
    }

    return QVariant();
}

QVariant PlatformsModel::headerData(int /*section*/,
                                    Qt::Orientation /*orientation*/,
                                    int /*role*/) const
{
    return QVariant();
}

PlatformsModel::PlatformsModel(lt::Platforms * platforms, QObject * parent)
    : QAbstractItemModel(parent), platforms_(platforms)
{
}
