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
#include "rom.h"

#include <cassert>

TableGroup::TableGroup(const std::shared_ptr<RomData> &base) : base_(base) {
    tables_.resize(base->rom()->platform()->tables.size());
}



size_t TableGroup::count() const
{
    return tables_.size();
}



Table *TableGroup::get(size_t idx, bool create) {
    assert(idx < tables_.size());

    std::unique_ptr<Table> &table = tables_[idx];
    Table *ptr;
    if (!table && create) {
        table = base_->loadTable(idx); //loadTable(*base_, idx);
        ptr = table.get();
        set(idx, std::move(table));
    } else {
        ptr = table.get();
    }

    return ptr;
}



void TableGroup::set(size_t idx, std::unique_ptr<Table> &&table) {
    assert(idx < tables_.size());
    
    QObject::connect(table.get(), &Table::onModified, [this]() {
        dirty_ = true;
    });

    tables_[idx] = std::move(table);
}



void TableGroup::apply(uint8_t *data, size_t size, Endianness endianness) {
    std::vector<uint8_t> res;
    std::size_t id = 0;
    for (const std::unique_ptr<Table> &table : tables_) {
        if (table && table->dirty()) {
            size_t offset = base_->rom()->model()->tables[id];
            assert(offset < size);
            table->serialize(data + offset, size - offset, endianness);
        }
        id++;
    }
}
