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

#include "log.h"

#include <QColor>

Log::Log() { connect(&Logger::get(), &Logger::appended, this, &Log::append); }

void Log::appendEntry(LogEntry &&entry) {
    beginInsertRows(QModelIndex(), entries_.size(), entries_.size());

    entries_.emplace_back(std::move(entry));

    endInsertRows();
}

void Log::append(Logger::Mode mode, const QString &text) {
    appendEntry({mode, text.toStdString()});
}

int Log::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return entries_.size();
}

QVariant Log::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.column() != 0) {
        return QVariant();
    }

    if (index.row() < 0 ||
        static_cast<std::size_t>(index.row()) >= entries_.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return QString::fromStdString(entries_[index.row()].text);
    }

    if (role == Qt::TextColorRole) {
        switch (entries_[index.row()].mode) {
        case Logger::Mode::Critical:
            return QColor::fromRgb(170, 0, 0);
        case Logger::Mode::Info:
            return QColor::fromRgb(255, 255, 255);
        case Logger::Mode::Debug:
            return QColor::fromRgb(185, 81, 255);
        case Logger::Mode::Warning:
            return QColor::fromRgb(229, 137, 25);
        }
    }

    return QVariant();
}
