#ifndef OVERBOOST_TABLEDETAILSMODEL_H
#define OVERBOOST_TABLEDETAILSMODEL_H

#include <QAbstractItemModel>
#include <rom/table.h>

class TableDetailsModel : public QAbstractItemModel
{
public:
    QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    QModelIndex parent(const QModelIndex & child) const override;
    int rowCount(const QModelIndex & parent) const override;
    int columnCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setTable(const lt::Table * table);

private:
    const lt::Table * table_{nullptr};
};

#endif // OVERBOOST_TABLEDETAILSMODEL_H
