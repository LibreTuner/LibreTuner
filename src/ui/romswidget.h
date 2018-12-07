#ifndef ROMSWIDGET_H
#define ROMSWIDGET_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QTreeView>

class RomStore;
class TuneManager;

class RomsModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit RomsModel(QObject *parent = nullptr);

    void setRoms(RomStore *roms);
    void setTunes(TuneManager *tunes);

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
    TuneManager *tunes_ = nullptr;


};


class RomsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RomsWidget(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

signals:

public slots:

private:
    QTreeView *treeView_;
};

#endif // ROMSWIDGET_H
