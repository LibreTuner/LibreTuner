#ifndef OVERBOOST_PLATFORMSMODEL_H
#define OVERBOOST_PLATFORMSMODEL_H

#include <QAbstractItemModel>
#include <definition/platform.h>

class PlatformsModel : public QAbstractItemModel
{
public:
    explicit PlatformsModel(const lt::Platforms * platforms, QObject * parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    QModelIndex parent(const QModelIndex & child) const override;
    int rowCount(const QModelIndex & parent) const override;
    int columnCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

private:
    const lt::Platforms * platforms_;
};

Q_DECLARE_METATYPE(const lt::Platform *)
Q_DECLARE_METATYPE(const lt::Model *)

#endif // OVERBOOST_PLATFORMSMODEL_H
