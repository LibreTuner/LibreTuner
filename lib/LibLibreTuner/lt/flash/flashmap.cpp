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

#include "flashmap.h"
#include "../definition/platform.h"
#include "../rom/rom.h"
#include "../rom/tableext.h"

#include <cassert>

namespace lt {

FlashMap::FlashMap(const std::vector<uint8_t> &data, std::size_t offset)
    : data_(data), offset_(offset) {}

FlashMap::FlashMap(std::vector<uint8_t> &&data, std::size_t offset)
    : data_(std::move(data)), offset_(offset) {}

FlashMap FlashMap::fromTune(Tune &tune) {
    const lt::RomPtr &rom = tune.base();

    const lt::ModelPtr &model = rom->model();
    const lt::Platform &platform = model->platform;

    std::size_t offset = platform.flashOffset;

    std::vector<uint8_t> data(rom->data() + offset,
                              rom->data() + rom->size() - offset);

    // Try each table
    for (const lt::ModelTable &modTable : model->tables) {
        if (modTable.table == nullptr) {
            continue;
        }

        const TableDefinition *tableDef = modTable.table;
        std::size_t tableOffset = modTable.offset;

        Table *table = tune.getTable(tableDef->id, false);
        if (table == nullptr) {
            continue;
        }

        std::vector<uint8_t> serialized = table->serialize(platform.endianness);

        std::copy(serialized.begin(), serialized.end(),
                  data.begin() + tableOffset - offset);
    }

    return FlashMap(std::move(data), offset);
}

} // namespace lt
