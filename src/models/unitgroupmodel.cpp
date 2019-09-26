#include "unitgroupmodel.h"

UnitGroupModel::UnitGroupModel(lt::UnitGroup * ug) {}

int UnitGroupModel::rowCount(const QModelIndex & parent) const
{
    if (unitGroup_ == nullptr)
        return 0;
    return unitGroup_->units().size();
}
QVariant UnitGroupModel::data(const QModelIndex & index, int role) const
{
    if (unitGroup_ == nullptr)
        return QVariant();
    const std::vector<lt::UnitName> & names = unitGroup_->units();
    if (index.row() >= names.size() || index.row() < 0 || index.column() != 1)
        return QVariant();
    return QString(names[index.row()].name);
}
