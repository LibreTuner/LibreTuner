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
#include "table.h"

#include "definition/platform.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <cassert>
#include <fstream>

namespace fs = std::filesystem;

namespace lt
{
namespace detail
{
EntriesPtr<double> createEntries(Endianness endianness, DataType dataType, const View & view)
{
    switch (endianness)
    {
    case Endianness::Big:
        return create_entries<double, Endianness::Big>(dataType, view);
    case Endianness::Little:
        return create_entries<double, Endianness::Big>(dataType, view);
    default:
        return EntriesPtr<double>();
    }
}
} // namespace detail

bool Tune::dirty() const noexcept
{
    for (const auto & [id, table] : tables_)
    {
        if (table->dirty())
            return true;
    }
    return false;
}

void Tune::clearDirty() noexcept
{
    for (const auto & [id, table] : tables_)
    {
        if (table->dirty())
            table->clearDirty();
    }
}

Table * Tune::getTable(const std::string & id, bool create)
{
    if (auto it = tables_.find(id); it != tables_.end())
        return it->second.get();

    // Else, try to create a new table from base calibration
    if (create)
    {
        const TableDefinition * def = base_->model()->getTable(id);
        if (def == nullptr)
            return nullptr;

        if (size() < def->offset.value() + def->byteSize())
        {
            throw std::runtime_error("table '" + id +
                                     "' could not be created because it "
                                     "exceeds the size of the ROM");
        }

        Table::Builder builder;
        builder.setBounds(def->minimum, def->maximum)
            .setName(def->name)
            .setDescription(def->description)
            .setScale(def->scale)
            .setSize(def->width, def->height);

        if (!def->axisX.empty())
            builder.setXAxis(getAxis(def->axisX, true));
        if (!def->axisY.empty())
            builder.setYAxis(getAxis(def->axisY, true));

        builder.setEntries(detail::createEntries(endianness(), def->dataType, data_.view(def->offset.value(), def->byteSize())));
        builder.setBaseEntries(detail::createEntries(endianness(), def->dataType, base_->view(def->offset.value(), def->byteSize())));

        // Emplace table and use returned iterator to get the inserted table
        return tables_.emplace(id, std::make_unique<Table>(builder.build())).first->second.get();
    }
    return nullptr;
}

AxisPtr Tune::getAxis(const std::string & id, bool create)
{
    // Check axes cache
    auto it = axes_.find(id);
    if (it != axes_.end())
        return it->second;

    if (!create)
        return AxisPtr();

    auto platform = base_->model()->platform();
    if (!platform)
        return AxisPtr();

    const lt::AxisDefinition * def = platform->getAxis(id);
    if (def == nullptr)
        return AxisPtr();

    Axis::Builder builder;
    builder.setName(def->name);

    std::visit(
        [&](auto && typeDefinition) {
            using T = std::decay_t<decltype(typeDefinition)>;
            if constexpr (std::is_same_v<T, LinearAxisDefinition>)
            {
                // Linear axis
                builder.setLinear(typeDefinition.start, typeDefinition.increment);
            }
            else if constexpr (std::is_same_v<T, MemoryAxisDefinition>)
            {
                // Memory axis
                // TODO: offsets should work the same as tables.
                int offset = base_->model()->getAxisOffset(id);
                int size = typeDefinition.size * static_cast<int>(dataTypeSize(def->dataType));
                if (offset + size > base_->size())
                {
                    throw std::runtime_error("axis exceeds rom size (rom size: " + std::to_string(base_->size()) +
                                             ", axis ends at " + std::to_string(offset + size) + ")");
                }

                View view = data_.view(offset, size);

                switch (endianness())
                {
                case Endianness::Big:
                    builder.setEntries(create_entries<double, Endianness::Big>(def->dataType, view));
                    break;
                case Endianness::Little:
                    builder.setEntries(create_entries<double, Endianness::Little>(def->dataType, view));
                }
            }
        },
        def->def);

    return axes_.emplace(id, std::make_shared<Axis>(builder.build())).first->second;
}

Tune::MetaData Tune::metadata() const noexcept
{
    MetaData md;
    md.name = name_;
    md.path = path_;
    if (base_)
        md.base = base_->path().filename().string();
    return md;
}

void Tune::save() const
{
    if (path_.empty())
        throw std::runtime_error("attempt to save ROM without a path");

    std::ofstream file(path_, std::ios::binary | std::ios::out);
    if (!file.is_open())
        throw std::runtime_error("failed to open tune file '" + path_.string() + "' for writing");

    cereal::BinaryOutputArchive archive(file);
    archive(metadata(), data_);
}

Tune::Tune(RomPtr rom) : Tune(rom, MemoryBuffer(rom->cbegin(), rom->cend())) {}

Tune::Tune(RomPtr rom, MemoryBuffer && data) : base_(std::move(rom)), data_(std::move(data))
{
    assert(base_);

    if (base_->size() != size())
        throw std::runtime_error("The base ROM and tune data size do not match (" + std::to_string(base_->size()) +
                                 " vs " + std::to_string(size()) + "). The tune or base ROM is corrupt.");
}

Rom::MetaData Rom::metadata() const noexcept
{
    MetaData md;
    md.name = name_;
    md.path = path_;
    if (model_)
    {
        md.model = model_->id;
        if (auto platform = model_->platform())
            md.platform = platform->id;
    }
    return md;
}

void Rom::save() const
{
    if (path_.empty())
        throw std::runtime_error("attempt to save ROM without a path");

    std::ofstream file(path_, std::ios::binary | std::ios::out);
    if (!file.is_open())
        throw std::runtime_error("failed to open ROM file '" + path_.string() + "' for writing");

    cereal::BinaryOutputArchive archive(file);
    archive(metadata(), data_);
}

} // namespace lt

// Declare cereal versions out of scope
CEREAL_CLASS_VERSION(lt::Rom::MetaData, 1)
CEREAL_CLASS_VERSION(lt::Tune::MetaData, 1)