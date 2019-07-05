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

#include <cassert>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>

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

    const lt::AxisDefinition * def = base_->model()->platform.getAxis(id);
    if (def == nullptr)
        return AxisPtr();

    Axis::Builder builder;

    std::visit(
        [&](auto && typeDefinition) {
            using T = std::decay_t<decltype(typeDefinition)>;
            if constexpr (std::is_same_v<T, LinearAxisDefinition>)
            {
                // Linear axis
                builder.setLinear(typeDefinition.start, typeDefinition.increment, typeDefinition.size).build();
            }
            else if constexpr (std::is_same_v<T, MemoryAxisDefinition>)
            {
                // Memory axis
                // TODO: offsets should work the same as tables.
                int offset = base_->model()->getAxisOffset(def->id);
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
                auto end = start + size;

                switch (def->dataType)
                {
                case DataType::Float:
                    builder.setEntries(fromBytes<float>(start, end, endianness()));
                    break;
                case DataType::Uint8:
                    builder.setEntries(fromBytes<uint8_t>(start, end, endianness()));
                    break;
                case DataType::Uint16:
                    builder.setEntries(fromBytes<uint16_t>(start, end, endianness()));
                    break;
                case DataType::Uint32:
                    builder.setEntries(fromBytes<uint32_t>(start, end, endianness()));
                    break;
                case DataType::Int8:
                    builder.setEntries(fromBytes<int8_t>(start, end, endianness()));
                    break;
                case DataType::Int16:
                    builder.setEntries(fromBytes<int16_t>(start, end, endianness()));
                    break;
                case DataType::Int32:
                    builder.setEntries(fromBytes<int32_t>(start, end, endianness()));
                    break;
                default:
                    break;
                }
            }
        },
        def->def);

    return axes_.emplace(id, std::make_shared<Axis>(builder.build())).first->second;
}

// Define constructs in global scope
struct RomConstruct
{
    std::string id;
    std::string name;
    std::string platformId;
    std::string modelId;
    std::vector<uint8_t> data;

    template <class Archive>
    void serialize(Archive & archive, std::uint32_t const version)
    {
        if (version != 1)
            throw std::runtime_error("invalid ROM version, expected 1");

        archive(id, name, platformId, modelId, data);
    }
};

struct TableConstruct
{
    std::string id;
    std::vector<uint8_t> data;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(id, data);
    }
};

struct TuneConstruct
{
    std::string id;
    std::string name;
    std::string baseId;
    std::vector<TableConstruct> tables;

    template <class Archive>
    void serialize(Archive & archive, std::uint32_t const version)
    {
        if (version != 1)
            throw std::runtime_error("invalid tune version, expected 1");

        archive(id, name, baseId, tables);
    }
};

RomPtr FileRomDatabase::getRom(const std::string & id)
{
    // Search the cache
    if (auto it = cache_.find(id); it != cache_.end())
    {
        // Check if the pointer has expired
        if (auto rom = it->second.lock())
            return rom;
    }

    std::ifstream file(base_ / id, std::ios::binary | std::ios::in);
    if (!file.is_open())
        return RomPtr();

    // Deserialize ROM with cereal
    cereal::BinaryInputArchive archive(file);
    RomConstruct construct;
    archive(construct);

    // Find the model
    ModelPtr model = platforms_.find(construct.platformId, construct.modelId);
    if (!model)
        throw std::runtime_error("Unknown platform and rom combination '" +
                                 construct.platformId + "' and '" +
                                 construct.modelId + "'");
    auto rom = std::make_shared<Rom>(model);
    rom->setId(construct.id);
    rom->setName(construct.name);
    rom->setData(std::move(construct.data));
    // Insert into cache
    cache_.emplace(construct.id, rom);
    return rom;
}

TunePtr FileRomDatabase::loadTune(const std::filesystem::path & path)
{
    std::ifstream file(path, std::ios::binary | std::ios::in);
    if (!file.is_open())
        return TunePtr();

    cereal::BinaryInputArchive archive(file);
    TuneConstruct construct;
    archive(construct);

    RomPtr rom = getRom(construct.baseId);
    if (!rom)
        throw std::runtime_error("unable to find ROM with id '" +
                                 construct.baseId + "'");

    auto tune = std::make_shared<Tune>(rom);
    tune->setId(construct.id);
    tune->setName(construct.name);

    for (const TableConstruct & table : construct.tables)
    {
        tune->setTable(table.id, table.data.data(), table.data.size());
    }
    return tune;
}

void FileRomDatabase::saveRom(const Rom & rom)
{
    RomConstruct construct;
    construct.name = rom.name();
    construct.id = rom.id();
    construct.modelId = rom.model()->id;
    construct.platformId = rom.model()->platform.id;
    construct.data = std::vector<uint8_t>(rom.data(), rom.data() + rom.size());

    trim(construct.id);
    if (construct.id.empty())
        throw std::runtime_error("attempt to save ROM with empty id");

    std::filesystem::path path = base_ / construct.id;
    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (!file.is_open())
        throw std::runtime_error("failed to open ROM file '" + path.string() + "' for writing");

    cereal::BinaryOutputArchive archive(file);
    archive(construct);
}

void FileRomDatabase::saveTune(const Tune & tune, std::filesystem::path & path)
{
    TuneConstruct construct;
    construct.id = tune.id();
    construct.name = tune.name();
    construct.baseId = tune.base()->id();
    for (auto &[id, table] : tune.tables())
    {
        TableConstruct tc;
        tc.id = id;
        tc.data = table->intoBytes(tune.endianness());
        construct.tables.emplace_back(std::move(tc));
    }

    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (!file.is_open())
        throw std::runtime_error("failed to open tune file '" + path.string() + "' for writing");

    cereal::BinaryOutputArchive archive(file);
    archive(construct);
}

} // namespace lt

// Declare cereal version out of scope
CEREAL_CLASS_VERSION(lt::Rom, 1)