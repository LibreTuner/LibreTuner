#include "serialportmodel.h"

QModelIndex SerialPortModel::parent(const QModelIndex &child) const {
    return QModelIndex();
}

int SerialPortModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return ports_.size();
}

int SerialPortModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

QVariant SerialPortModel::data(const QModelIndex &index, int role) const {
    if (role != Qt::DisplayRole && role != Qt::UserRole) {
        return QVariant();
    }

    if (index.column() != 0 || index.row() >= ports_.size()) {
        return QVariant();
    }

    return QString::fromStdString(ports_[index.row()]);
}

SerialPortModel::SerialPortModel(QObject *parent) : QAbstractListModel(parent) {

}
