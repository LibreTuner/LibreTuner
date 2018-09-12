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

#include "table.h"
#include "util.hpp"
/*
Table::Table(const TableDefinition *definition) : definition_(definition) {}

Qt::ItemFlags Table::flags(const QModelIndex &index) const {
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QVariant Table::headerData(int section, Qt::Orientation orientation,
                           int role) const {
    if (role != Qt::DisplayRole || section < 0) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        if (definition_->axisX() != nullptr && section < definition_->sizeX()) {
            return QString::number(definition_->axisX()->label(section));
        }
    } else if (orientation == Qt::Vertical) {
        if (definition_->axisY() != nullptr && section < definition_->sizeY()) {
            return QString::number(definition_->axisY()->label(section));
        }
    }

    return QVariant();
}

template <template <class Type> class TemplateType> class TCreator {
public:
    static std::shared_ptr<Table> create(DataType dt,
                                         const TableDefinition *def,
                                         Endianness endian,
                                         gsl::span<uint8_t> data) {
        switch (dt) {
        case TDATA_FLOAT:
            return std::make_shared<TemplateType<float>>(def, endian, data);
        case TDATA_INT32:
            return std::make_shared<TemplateType<int32_t>>(def, endian, data);
        case TDATA_INT16:
            return std::make_shared<TemplateType<int16_t>>(def, endian, data);
        case TDATA_INT8:
            return std::make_shared<TemplateType<int8_t>>(def, endian, data);
        case TDATA_UINT8:
            return std::make_shared<TemplateType<uint8_t>>(def, endian, data);
        case TDATA_UINT16:
            return std::make_shared<TemplateType<uint16_t>>(def, endian, data);
        case TDATA_UINT32:
            return std::make_shared<TemplateType<uint32_t>>(def, endian, data);
        }
        return nullptr;
    }
};

std::shared_ptr<Table> Table::create(TableType tableType, DataType dataType,
                                     const TableDefinition *def,
                                     Endianness endian,
                                     gsl::span<uint8_t> data) {
    switch (tableType) {
    case TABLE_1D:
        return TCreator<Table1d>::create(dataType, def, endian, data);
    case TABLE_2D:
        return TCreator<Table2d>::create(dataType, def, endian, data);
    default:
        assert(false && "unimplemented");
    }
    return nullptr;
}
*/
