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

#include "tablegroup.h"
#include "definitions/definition.h"
#include "table.h"

#include <cassert>

TableGroup::TableGroup(const std::shared_ptr<Rom> &base) : base_(base) {
    tables_.resize(base->definition()->tables.size());
}



size_t TableGroup::count() const
{
    return tables_.size();
}



Table *TableGroup::get(size_t idx, bool create) {
    assert(idx < tables_.size());

    std::unique_ptr<Table> &table = tables_[idx];
    if (!table && create) {
        table = loadTable(*base_, idx);
    }

    return table.get();
}



void TableGroup::set(size_t idx, std::unique_ptr<Table> &&table) {
    assert(idx < tables_.size());
    
    tables_[idx] = std::move(table);

    /*
    const TableDefinition *definition = base_->definition()->tables()->at(idx);

    TableType type = definition->type();
    DataType dataType = definition->dataType();

    TablePtr table;

    switch (type) {
    case TABLE_1D:
        switch (dataType) {
        case TDATA_FLOAT:
            if (data.size() != (definition->sizeX() * sizeof(float))) {
                throw std::runtime_error("invalid table size");
            }
            table = std::make_shared<Table1d<float>>(definition,
                                                     Endianness::Big, data);
            break;
        }
        break;
    case TABLE_2D:
        switch (dataType) {
        case TDATA_FLOAT:
            if (data.size() !=
                (definition->sizeX() * definition->sizeY() * sizeof(float))) {
                throw std::runtime_error("invalid table size");
            }
            table = std::make_shared<Table2d<float>>(definition,
                                                     Endianness::Big, data);
            break;
        }

        break;
    case TABLE_3D:
        break;
    }

    if (table) {
        table->setModified(true);
        table->calcDifference(base_->getTable(idx));
        tables_[idx] = table;
    }*/
}



void TableGroup::apply(gsl::span<uint8_t> data, Endianness endianness) {
    std::vector<uint8_t> res;
    std::size_t id = 0;
    for (const std::unique_ptr<Table> &table : tables_) {
        if (table && table->dirty()) {
            size_t offset = base_->definition()->tables[id];
            assert(offset < data.size());
            table->serialize(data.subspan(offset), endianness);
        }
        id++;
    }
}
