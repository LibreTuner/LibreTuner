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

#include <cassert>

namespace lt
{

FlashMap::FlashMap(const std::vector<uint8_t> & data, std::size_t offset)
    : data_(data), offset_(offset)
{
}

FlashMap::FlashMap(std::vector<uint8_t> && data, std::size_t offset)
    : data_(std::move(data)), offset_(offset)
{
}

FlashMap FlashMap::fromTune(Tune & tune)
{
    const lt::RomPtr & rom = tune.base();

    const lt::ModelPtr & model = rom->model();
    lt::PlatformPtr platform = model->platform();
    if (!platform)
        throw std::runtime_error("model does not have a valid platform. (did "
                                 "the reference expire?)");

    std::size_t offset = platform->flashOffset;

    std::vector<uint8_t> data(rom->data() + offset,
                              rom->data() + rom->size() - offset);

    // Try each table
    for (const auto & [id, definition] : model->tables)
    {
        Table * table = tune.getTable(id, false);
        if (table == nullptr)
        {
            continue;
        }

        std::vector<uint8_t> serialized =
            table->intoBytes(platform->endianness);

        std::copy(serialized.begin(), serialized.end(),
                  data.begin() + definition.offset.value() - offset);
    }

    return FlashMap(std::move(data), offset);
}

} // namespace lt
