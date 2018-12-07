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
    tables_.resize(base->platform()->tables.size());
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
}



void TableGroup::apply(uint8_t *data, size_t size, Endianness endianness) {
    std::vector<uint8_t> res;
    std::size_t id = 0;
    for (const std::unique_ptr<Table> &table : tables_) {
        if (table && table->dirty()) {
            size_t offset = base_->model()->tables[id];
            assert(offset < size);
            table->serialize(data + offset, size - offset, endianness);
        }
        id++;
    }
}
