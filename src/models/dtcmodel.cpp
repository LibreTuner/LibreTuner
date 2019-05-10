#include "dtcmodel.h"

#include "libretuner.h"

int DtcModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return 0;
    }
    return codes_.size();
}

int DtcModel::columnCount(const QModelIndex & /*parent*/) const { return 2; }

QVariant DtcModel::data(const QModelIndex &index, int role) const {
    if (role != Qt::DisplayRole || !index.isValid() || index.column() < 0 ||
        index.column() > 1) {
        return QVariant();
    }

    if (index.row() >= static_cast<int>(codes_.size()) || index.row() < 0) {
        return QVariant();
    }

    std::string codeString = codes_[index.row()].codeString();

    switch (index.column()) {
    case 0:
        // Code
        return QString::fromStdString(codeString);
    case 1:
        // Description
        auto desc = LT()->dtcDescriptions().get(codeString);
        if (!desc.first) {
            return tr("Unknown");
        }
        return QString::fromStdString(desc.second);
    }
    return QVariant();
}

QVariant DtcModel::headerData(int section, Qt::Orientation orientation,
                              int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("Code");
    case 1:
        return tr("Description");
    default:
        break;
    }
    return QVariant();
}
