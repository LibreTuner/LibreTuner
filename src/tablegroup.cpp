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

TableGroup::TableGroup(const RomDataPtr &base) : base_(base) {
  tables_.resize(base->definition()->tables()->count());
}

TablePtr TableGroup::get(size_t idx, bool create) {
  assert(idx < tables_.size());

  TablePtr &table = tables_[idx];
  if (!table && create) {
    table = base_->getTable(idx);
  }

  return table;
}

std::pair<bool, std::string> TableGroup::set(size_t idx, const uint8_t *data,
                                             size_t size) {
  assert(idx < tables_.size());
  assert(size >= 0);

  const TableDefinition *definition = base_->definition()->tables()->at(idx);

  TableType type = definition->type();
  DataType dataType = definition->dataType();

  TablePtr table;

  switch (type) {
  case TABLE_1D:
    switch (dataType) {
    case TDATA_FLOAT:
      if (size != (definition->sizeX() * sizeof(float))) {
        return std::make_pair(false, "Invalid table size");
      }
      table = std::make_shared<Table1d<float>>(definition, ENDIAN_BIG, data);
      break;
    }
    break;
  case TABLE_2D:
    switch (dataType) {
    case TDATA_FLOAT:
      if (size != (definition->sizeX() * definition->sizeY() * sizeof(float))) {
        return std::make_pair(false, "Invalid table size");
      }
      table = std::make_shared<Table2d<float>>(definition, ENDIAN_BIG, data);
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
  }

  return std::make_pair(true, "");
}

void TableGroup::apply(uint8_t *data, size_t length) {
  std::vector<uint8_t> res;
  for (TablePtr table : tables_) {
    if (table && table->modified()) {
      size_t offset =
          base_->subDefinition()->getTableLocation(table->definition()->id());
      assert(offset < length);
      assert(table->serialize(data + offset, length - offset));
    }
  }
}
