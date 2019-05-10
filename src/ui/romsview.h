#ifndef ROMSWIDGET_H
#define ROMSWIDGET_H

#include <QAbstractItemModel>
#include <QTreeView>
#include <QWidget>

#include <memory>

class Tune;

class RomStore;

class RomsModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit RomsModel(QObject *parent = nullptr);

    void setRoms(RomStore *roms);

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;

private:
    RomStore *roms_ = nullptr;
};

class RomsView : public QTreeView {
    Q_OBJECT
public:
    explicit RomsView(QWidget *parent = nullptr);

    Tune *selectedTune();

private slots:
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;

signals:
    void tuneChanged();
};

#endif // ROMSWIDGET_H
