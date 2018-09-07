/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "canlog.h"

void CanLog::addMessage(const CanMessage &message) {
    std::lock_guard<std::mutex> guard(mutex_);
    beginInsertRows(QModelIndex(), messages_.size(), messages_.size());
    messages_.push_back(message);
    endInsertRows();
}

CanLog::~CanLog() { emit destructing(); }

QVariant CanLog::headerData(int section, Qt::Orientation orientation,
                            int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0:
                return QVariant("ID");
            case 1:
                return QVariant("Length");
            case 2:
                return QVariant("Message");
            }
        }
    }

    return QVariant();
}

int CanLog::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;

    return size();
}

int CanLog::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;

    return 3;
}

QVariant CanLog::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= size()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    std::lock_guard<std::mutex> guard(mutex_);
    const CanMessage &message = messages_[index.row()];

    switch (index.column()) {
    case 0:
        return QVariant(QString::number(message.id(), 16));
    case 1:
        return QVariant(message.length());
    case 2:
        return QVariant(QString::fromStdString(message.strMessage()));
    }

    return QVariant();
}
