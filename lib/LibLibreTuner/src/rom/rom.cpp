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

#include "rom.h"
#include "tableext.h"

#include "definition/platform.h"

#include <cassert>

namespace lt {

Rom::Rom() = default;

TablePtr Rom::baseTable(std::size_t tableId) const {
    if (tableId >= model_->tables.size()) {
        return nullptr;
    }

    const ModelTable &modTable = model_->getTable(tableId);
    std::size_t offset = modTable.offset;
	const TableDefinition &tabDef = modTable.table;

	TablePtr table = std::make_unique<Table>();

	TableDeserializer td(model_->platform.endianness);
	//td.deserialize<float>(*table.get(), tableDef.width, tableDef.height, data_.data() + offset, data_.size(), tableDef.scale);

	// TODO: Load axes.....

	return table;
}

} // namespace lt