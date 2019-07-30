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

        if (base_->size() < def->offset.value() + def->byteSize())
        {
            throw std::runtime_error("table '" + id +
                                     "' could not be created because it "
                                     "exceeds the size of the ROM");
        }
        auto data = base_->cbegin();
        std::advance(data, def->offset.value());
        return setTable(id, &*data, def->byteSize());
    }
    return nullptr;
}

Table * Tune::setTable(const std::string & id, const uint8_t * data,
                       std::size_t length)
{
    const TableDefinition * def = base_->model()->getTable(id);
    if (def == nullptr)
        return nullptr;

    Table::Builder builder;
    builder.setBounds(def->minimum, def->maximum)
        .setName(def->name)
        .setScale(def->scale)
        .setSize(def->width, def->height);

    if (!def->axisX.empty())
        builder.setXAxis(getAxis(def->axisX, true));
    if (!def->axisY.empty())
        builder.setYAxis(getAxis(def->axisY, true));

    // Deserialize
    auto begin = data;
    auto end = &data[length];
    datatypeToType(def->dataType, builder, begin, end, endianness());

    // Emplace table and use returned iterator to get the inserted table
    return tables_.emplace(id, std::make_unique<Table>(builder.build()))
        .first->second.get();
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
                builder
                    .setLinear(typeDefinition.start, typeDefinition.increment,
                               typeDefinition.size);
            }
            else if constexpr (std::is_same_v<T, MemoryAxisDefinition>)
            {
                // Memory axis
                // TODO: offsets should work the same as tables.
                int offset = base_->model()->getAxisOffset(id);
                int size = typeDefinition.size;
                if (offset +
                        size * static_cast<int>(dataTypeSize(def->dataType)) >
                    base_->size())
                {
                    throw std::runtime_error(
                        "axis exceeds rom size (rom size: " +
                        std::to_string(base_->size()) + ", axis ends at " +
                        std::to_string(offset + size) + ")");
                }

                auto start = std::next(base_->data(), offset);
                auto end = std::next(start, size * static_cast<int>(dataTypeSize(def->dataType)));

                switch (def->dataType)
                {
                case DataType::Float:
                    builder.setEntries(
                        fromBytes<float>(start, end, endianness()));
                    break;
                case DataType::Uint8:
                    builder.setEntries(
                        fromBytes<uint8_t>(start, end, endianness()));
                    break;
                case DataType::Uint16:
                    builder.setEntries(
                        fromBytes<uint16_t>(start, end, endianness()));
                    break;
                case DataType::Uint32:
                    builder.setEntries(
                        fromBytes<uint32_t>(start, end, endianness()));
                    break;
                case DataType::Int8:
                    builder.setEntries(
                        fromBytes<int8_t>(start, end, endianness()));
                    break;
                case DataType::Int16:
                    builder.setEntries(
                        fromBytes<int16_t>(start, end, endianness()));
                    break;
                case DataType::Int32:
                    builder.setEntries(
                        fromBytes<int32_t>(start, end, endianness()));
                    break;
                default:
                    break;
                }
            }
        },
        def->def);

    return axes_.emplace(id, std::make_shared<Axis>(builder.build()))
        .first->second;
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

    TuneConstruct construct;
    construct.meta = metadata();

    for (auto & [id, table] : tables_)
    {
        TableConstruct tc;
        tc.id = id;
        tc.data = table->intoBytes(endianness());
        construct.tables.emplace_back(std::move(tc));
    }

    std::ofstream file(path_, std::ios::binary | std::ios::out);
    if (!file.is_open())
        throw std::runtime_error("failed to open tune file '" + path_.string() +
                                 "' for writing");

    cereal::BinaryOutputArchive archive(file);
    archive(construct);
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
    RomConstruct construct;
    construct.meta = metadata();
    construct.data = std::vector<uint8_t>(data_);

    std::ofstream file(path_, std::ios::binary | std::ios::out);
    if (!file.is_open())
        throw std::runtime_error("failed to open ROM file '" + path_.string() +
                                 "' for writing");

    cereal::BinaryOutputArchive archive(file);
    archive(construct);
}


} // namespace lt

// Declare cereal versions out of scope
CEREAL_CLASS_VERSION(lt::Rom::MetaData, 1)
CEREAL_CLASS_VERSION(lt::Tune::MetaData, 1)