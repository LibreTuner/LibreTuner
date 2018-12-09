#include "romsview.h"
#include "rommanager.h"

#include "logger.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>

RomsView::RomsView (QWidget *parent) : QTreeView(parent)
{
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setStretchLastSection(true);
}


void RomsView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    emit tuneChanged();
    QTreeView::selectionChanged(selected, deselected);
}



Tune *RomsView::selectedTune()
{
    QModelIndexList indicies = selectedIndexes();
    if (indicies.isEmpty()) {
        return nullptr;
    }
    QVariant var = model()->data(indicies.front(), Qt::UserRole);
    if (var.canConvert<Tune*>()) {
        return var.value<Tune*>();
    }
    return nullptr;
}



RomsModel::RomsModel(QObject *parent) : QAbstractItemModel(parent), roms_(RomStore::get())
{

}



void RomsModel::setRoms(RomStore *roms)
{
    roms_ = roms;
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

    return roms[rom_id]->tunes().size();
}



int RomsModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

Q_DECLARE_METATYPE(Tune*)

QVariant RomsModel::data(const QModelIndex &index, int role) const
{
    
    if (!roms_) {
        return QVariant();
    }

    if (index.internalId() == 0) {
        if (role != Qt::DisplayRole) {
            return QVariant();
        }
        // Rom
        Rom *rom = roms_->get(index.row());
        if (rom == nullptr) {
            return QVariant();
        }
        
        if (index.column() == 0) {
            return QVariant(QString::fromStdString(rom->name()));
        } else if (index.column() == 1) {
            return QVariant(QString::fromStdString(rom->platform()->id));
        } else if (index.column() == 2) {
            return QVariant(QString::fromStdString(rom->model()->id));
        } else {
            return QVariant();
        }
    }

    // Tune

    if (index.column() != 0) {
        return QVariant();
    }

    std::size_t rom_id = index.internalId() - 1;
    
    Rom *rom = roms_->get(rom_id);
    if (rom == nullptr) {
        return QVariant();
    }
        
    if (index.row() >= rom->tunes().size()) {
        return QVariant();
    }
    
    Tune *tune = rom->getTune(index.row());
    if (tune == nullptr) {
        Logger::info("Invalid tune id: " + std::to_string(index.row()));
        return QVariant();
    }
    
    switch (role) {
        case Qt::DisplayRole:
            return QVariant(QString::fromStdString(tune->name()));
        case Qt::UserRole:
            return QVariant::fromValue<Tune*>(tune);
        default:
            return QVariant();
    }
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
