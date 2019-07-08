#ifndef LIBRETUNER_PROJECTS_H
#define LIBRETUNER_PROJECTS_H

#include <QAbstractItemModel>

#include <lt/project/project.h>

class TreeItem;

class Projects : public QAbstractItemModel
{
public:
    explicit Projects(QObject * parent = nullptr);
    ~Projects() override;

    void addProject(lt::ProjectPtr project);


    QModelIndex index(int row, int column,
                      const QModelIndex & parent) const override;
    QModelIndex parent(const QModelIndex & child) const override;
    int rowCount(const QModelIndex & parent) const override;
    int columnCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;

private:
    TreeItem * root_;
};

#endif // LIBRETUNER_PROJECTS_H
