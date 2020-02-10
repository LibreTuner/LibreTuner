#ifndef LIBRETUNER_UNITGROUPMODEL_H
#define LIBRETUNER_UNITGROUPMODEL_H

#include <QAbstractListModel>
#include <lt/rom/unit.h>

class UnitGroupModel : public QAbstractListModel
{
public:
    explicit UnitGroupModel(lt::UnitGroup * ug);

    QVariant data(const QModelIndex & index, int role) const override;
    int rowCount(const QModelIndex & parent) const override;

    void setUnitGroup(lt::UnitGroup * ug) noexcept
    {
        beginResetModel();
        unitGroup_ = ug;
        endResetModel();
    }

private:
    lt::UnitGroup * unitGroup_{nullptr};
};

#endif // LIBRETUNER_UNITGROUPMODEL_H
