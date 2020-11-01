#ifndef OVERBOOST_PIDSMODEL_H
#define OVERBOOST_PIDSMODEL_H

#include <QAbstractItemModel>
#include <datalog/pid.h>

namespace lt {
struct Platform;
}

class PidsModel : public QAbstractItemModel
{
public:
    explicit PidsModel(const lt::Platform * platform = nullptr, QObject * parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    QModelIndex parent(const QModelIndex & child) const override;
    int rowCount(const QModelIndex & parent) const override;
    int columnCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;

    void setPlatform(const lt::Platform * platform);

private:
    const lt::Platform * platform_;

    QList<const lt::Pid*> selected_;
};

#endif // OVERBOOST_PIDSMODEL_H