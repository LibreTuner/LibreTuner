#ifndef ROMSWIDGET_H
#define ROMSWIDGET_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QTreeView>

#include <memory>

class Tune;

class RomStore;

class RomsModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit RomsModel(QObject *parent = nullptr);

    void setRoms(RomStore *roms);
    
    // Returns the tune from an index or nullptr if no tune is associated with the index
    std::shared_ptr<Tune> getTune(const QModelIndex &index) const;

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    RomStore *roms_ = nullptr;
};


class RomsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RomsWidget(QWidget *parent = nullptr);

    void setModel(RomsModel *model);

signals:
    void activated(const std::shared_ptr<Tune> &tune);
    void downloadClicked();
    void flashClicked();

public slots:

private:
    QTreeView *treeView_;
    RomsModel *model_ = nullptr;
};

#endif // ROMSWIDGET_H
