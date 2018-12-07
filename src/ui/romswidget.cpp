#include "romswidget.h"
#include "rommanager.h"
#include "tunemanager.h"

#include "logger.h"

#include <QVBoxLayout>
#include <QHeaderView>

RomsWidget::RomsWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    treeView_ = new QTreeView(this);

    treeView_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeView_->header()->setStretchLastSection(true);

    layout->addWidget(treeView_);
    setLayout(layout);
}



void RomsWidget::setModel(QAbstractItemModel *model)
{
    treeView_->setModel(model);
}



RomsModel::RomsModel(QObject *parent) : QAbstractItemModel(parent)
{

}



void RomsModel::setRoms(RomStore *roms)
{
    roms_ = roms;
}

void RomsModel::setTunes(TuneManager *tunes)
{
    tunes_ = tunes;
}



QModelIndex RomsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!roms_) {
        return QModelIndex();
    }
    if (!parent.isValid()) {
        return createIndex(row, column);
    }

    if (parent.internalId() == 0) {
        return createIndex(row, column, static_cast<quintptr>(parent.row() + 1));
    }

    // Tunes have no children
    return QModelIndex();
}



QModelIndex RomsModel::parent(const QModelIndex &child) const
{
    if (!roms_) {
        return QModelIndex();
    }
    if (!child.isValid()) {
        return QModelIndex();
    }

    if (child.internalId() == 0) {
        // Root node
        return QModelIndex();
    }

    int rom_id = child.internalId() - 1;
    return createIndex(rom_id, 0);
}



int RomsModel::rowCount(const QModelIndex &parent) const
{
    if (!roms_) {
        return 0;
    }
    if (!parent.isValid()) {
        return roms_->roms().size();
    }

    if (parent.internalId() != 0) {
        return 0;
    }

    std::size_t rom_id = parent.row();
    auto &roms = roms_->roms();
    if (rom_id >= roms.size()) {
        return 0;
    }

    return roms[rom_id].tunes.size();
}



int RomsModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}



QVariant RomsModel::data(const QModelIndex &index, int role) const
{
    if (!roms_) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    auto &roms = roms_->roms();

    if (index.internalId() == 0) {
        // Rom
        if (index.row() >= roms.size()) {
            return QVariant();
        }
        const RomMeta &rom = roms[index.row()];
        if (index.column() == 0) {
            return QVariant(QString::fromStdString(rom.name));
        } else if (index.column() == 1) {
            return QVariant(QString::fromStdString(rom.definitionId));
        } else if (index.column() == 2) {
            return QVariant(QString::fromStdString(rom.modelId));
        } else {
            return QVariant();
        }
    }

    // Tune

    if (index.column() != 0) {
        return QVariant();
    }

    std::size_t rom_id = index.internalId() - 1;
    if (rom_id >= roms.size()) {
        return QVariant();
    }
    const RomMeta &rom = roms[rom_id];
    if (index.row() >= rom.tunes.size()) {
        return QVariant();
    }

    // Find tune id
    if (!tunes_) {
        return QVariant();
    }

    std::size_t tune_id = rom.tunes[index.row()];
    if (tune_id >= tunes_->tunes().size()) {
        Logger::info("Invalid id: " + std::to_string(tune_id));
        return QVariant();
    }

    return QVariant(QString::fromStdString(tunes_->tunes()[tune_id].name));
}



QVariant RomsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return QVariant("Name");
    case 1:
        return QVariant("Platform");
    case 2:
        return QVariant("Model");
    default:
        return QVariant();
    }
}
