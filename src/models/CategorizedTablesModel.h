#ifndef OVERBOOST_CATEGORIZEDTABLESMODEL_H
#define OVERBOOST_CATEGORIZEDTABLESMODEL_H

#include <QAbstractItemModel>
#include <definition/model.h>

struct TableCategory
{
    std::string name;
    std::vector<const lt::TableDefinition *> tables;
};

class CategorizedTablesModel : public QAbstractItemModel
{
public:
    explicit CategorizedTablesModel(const lt::Model * model = nullptr, QObject * parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    QModelIndex parent(const QModelIndex & child) const override;
    int rowCount(const QModelIndex & parent) const override;
    int columnCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setDefinition(const lt::Model * model);

private:
    const lt::Model * model_;

    std::vector<TableCategory> categories_;
};

Q_DECLARE_METATYPE(const lt::TableDefinition *)

#endif // OVERBOOST_CATEGORIZEDTABLESMODEL_H
