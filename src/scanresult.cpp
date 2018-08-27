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


#include <cassert>
#include "scanresult.h"

ScanResult::ScanResult()
{

}

int ScanResult::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return codes_.size();
}

int ScanResult::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 2;
}

QVariant ScanResult::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (index.row() < 0 || index.row() >= codes_.size()) {
        return QVariant();
    }

    switch (index.column()) {
    case 0:
        return QString::fromStdString(codes_[index.row()].codeString());
    case 1:
        return QString::fromStdString(codes_[index.row()].description);
    default:
        return QVariant();
    }

    assert("This should not be called");
}

QVariant ScanResult::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (section == 0) {
        return "Code";
    } else if (section == 1) {
        return "Description";
    }
    return QVariant();
}

static char firstDtc[4] = {'P', 'C', 'B', 'U'};

static char numericChar(uint8_t c) {
    return static_cast<char>(c) + '0';
}

std::string DiagnosticCode::codeString() const
{
    std::string res;
    res += firstDtc[(code & 0xC000) >> 14];
    res += numericChar((code & 0x3000) >> 12);
    res += numericChar((code & 0x0F00) >> 8);
    res += numericChar((code & 0x00F0) >> 4);
    res += numericChar(code & 0x000F);
    return res;
}
