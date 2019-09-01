#include "platform.h"
#include "../support/util.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace nlohmann
{
template<>
struct adl_serializer<lt::ChecksumPtr>
{
    static lt::ChecksumPtr from_json(const json & j)
    {
        const auto & mode = j.at("mode").get<std::string>();
        const auto offset = j.at("offset").get<std::size_t>();
        const auto size = j.at("size").get<std::size_t>();
        const auto target = j.at("target").get<std::size_t>();

        lt::ChecksumPtr sum;
        if (mode == "basic")
            sum = lt::ChecksumPtr(new lt::ChecksumBasic(offset, size, target));
        else
            throw std::runtime_error("invalid mode for checksum");

        if (auto it = j.find("modify"); it != j.end())
        {
            for (auto & section : *it)
            {
                sum->addModifiable(section.at("offset").get<std::size_t>(),
                                   section.at("size").get<std::size_t>());
            }
        }

        return sum;
    }
};
}

namespace lt
{

void to_json(json & j, const lt::Pid & pid)
{
    j = json{{"name", pid.name},
             {"description", pid.description},
             {"code", pid.code},
             {"formula", pid.formula},
             {"unit", pid.unit}};
}

void from_json(const json & j, lt::Pid & pid)
{
    j.at("name").get_to(pid.name);
    j.at("description").get_to(pid.description);
    j.at("code").get_to(pid.code);
    j.at("formula").get_to(pid.formula);
    j.at("unit").get_to(pid.unit);
}

NLOHMANN_JSON_SERIALIZE_ENUM(DataType, {
    {DataType::Invalid, nullptr},
    {DataType::Float, "float"},
    {DataType::Int8, "int8"},
    {DataType::Int16, "int16"},
    {DataType::Int32, "int32"},
    {DataType::Uint8, "uint8"},
    {DataType::Uint16, "uint16"},
    {DataType::Uint32, "uint32"},
})

void from_json(const json & j, lt::TableDefinition & table)
{
    j.at("name").get_to(table.name);
    j.at("description").get_to(table.description);
    if (auto it = j.find("unit"); it != j.end())
        it->get_to(table.unit);

    if (auto it = j.find("category"); it != j.end())
        it->get_to(table.category);

    j.at("datatype").get_to(table.dataType);
    if (table.dataType == DataType::Invalid)
        throw std::runtime_error("invalid datatype in table " + table.name);

    if (auto it = j.find("storeddatatype"); it != j.end())
        it->get_to(table.storedDataType);
    else
        table.storedDataType = table.dataType;

    if (table.storedDataType == DataType::Invalid)
        throw std::runtime_error("invalid storeddatatype in table " + table.name);

    if (auto it = j.find("width"); it != j.end())
        it->get_to(table.width);
    if (auto it = j.find("height"); it != j.end())
        it->get_to(table.height);

    if (auto it = j.find("axisx"); it != j.end())
        it->get_to(table.axisX);
    if (auto it = j.find("axisy"); it != j.end())
        it->get_to(table.axisY);
    if (auto it = j.find("scale"); it != j.end())
        it->get_to(table.scale);

    if (auto it = j.find("minimum"); it != j.end())
        it->get_to(table.minimum);
    if (auto it = j.find("maximum"); it != j.end())
        it->get_to(table.maximum);
}

void from_json(const json & j, lt::AxisDefinition & axis)
{
    j.at("name").get_to(axis.name);
    j.at("datatype").get_to(axis.dataType);

    auto type = j.at("type").get<std::string>();
    if (type == "memory")
    {
        lt::MemoryAxisDefinition memory;
        j.at("size").get_to(memory.size);
        axis.def.emplace<lt::MemoryAxisDefinition>(std::move(memory));
    }
    else if (type == "linear")
    {
        lt::LinearAxisDefinition linear;
        j.at("minimum").get_to(linear.start);
        j.at("increment").get_to(linear.increment);
        j.at("size").get_to(linear.size);
        axis.def.emplace<lt::LinearAxisDefinition>(std::move(linear));
    }
    else
        throw std::runtime_error("invalid axis type '" + type + "'");
}

NLOHMANN_JSON_SERIALIZE_ENUM(Endianness, {
    {Endianness::Big, "big"},
    {Endianness::Little, "little"},
})

void from_json(const json & j, lt::Platform & platform)
{
    j.at("id").get_to(platform.id);
    j.at("name").get_to(platform.name);
    j.at("romsize").get_to(platform.romsize);
    j.at("baudrate").get_to(platform.baudrate);
    if (auto it = j.find("logmode"); it != j.end())
    {
        it->get_to(platform.logMode);
        lt::lowercase_string(platform.logMode);
    }

    if (auto it = j.find("endianness"); it != j.end())
        it->get_to(platform.endianness);

    // Transfer
    if (auto transfer = j.find("transfer"); transfer != j.end())
    {
        if (auto it = transfer->find("flashmode"); it != transfer->end())
        {
            it->get_to(platform.flashMode);
            lt::lowercase_string(platform.flashMode);
        }
        if (auto it = transfer->find("downloadmode"); it != transfer->end())
        {
            it->get_to(platform.downloadMode);
            lt::lowercase_string(platform.downloadMode);
        }
        transfer->at("serverid").get_to(platform.serverId);
    }

    // Authentication
    if (auto auth = j.find("auth"); auth != j.end())
    {
        // Key should be the same for both
        if (auto it = auth->find("key"); it != auth->end())
        {
            it->get_to(platform.downloadAuthOptions.key);
            platform.flashAuthOptions.key = platform.downloadAuthOptions.key;
        }

        // Session IDs
        if (auto it = auth->find("sessionid"); it != auth->end())
        {
            it->get_to(platform.downloadAuthOptions.session);
            platform.flashAuthOptions.session =
                platform.downloadAuthOptions.session;
        }
        if (auto it = auth->find("download_sessionid"); it != auth->end())
            it->get_to(platform.downloadAuthOptions.session);
        if (auto it = auth->find("flash_sessionid"); it != auth->end())
            it->get_to(platform.flashAuthOptions.session);
    }

    // VIN patterns
    for (const auto & vin : j.at("vins"))
    {
        platform.vins.emplace_back(vin.get<std::string>());
    }

    if (auto axes = j.find("axes"); axes != j.end())
    {
        axes->get_to(platform.axes);
        /*
        for (auto it = axes.begin(); it != axes.end(); ++it)
        {
            platform.axes.emplace(it->first.as<std::string>(),
                                  it->second.as<lt::AxisDefinition>());
        }
         */
    }

    // Tables
    if (auto tables = j.find("tables"); tables != j.end())
    {
        for (auto & [id, table] : tables->items())
        {
            auto def = table.get<lt::TableDefinition>();
            def.id = id;
            platform.tables.emplace(id, std::move(def));
        }
    }

    if (auto it = j.find("pids"); it != j.end())
        it->get_to(platform.pids);
}

const TableDefinition * Platform::getTable(const std::string & id) const
    noexcept
{
    if (auto it = tables.find(id); it != tables.end())
        return &it->second;
    return nullptr;
}

ModelPtr Platform::findModel(const std::string & id) const noexcept
{
    for (const ModelPtr & model : models)
    {
        if (model->id == id)
            return model;
    }
    return nullptr;
}

const AxisDefinition * Platform::getAxis(const std::string & id) const noexcept
{
    if (auto it = axes.find(id); it != axes.end())
        return &it->second;
    return nullptr;
}

ModelPtr Platform::identify(const uint8_t * data, size_t size) const noexcept
{
    for (const ModelPtr & model : models)
    {
        if (model->isModel(data, size))
            return model;
    }
    return ModelPtr();
}

const Pid * Platform::getPid(uint32_t code) const noexcept
{
    for (const Pid & pid : pids)
    {
        if (pid.code == code)
            return &pid;
    }
    return nullptr;
}

PlatformPtr load_main(const fs::path & path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("file '" + path.string() +
                                 "' does not exist or LibreTuner does not have "
                                 "permission to open it.");
    }

    json root;
    file >> root;
    return std::make_shared<Platform>(root.get<Platform>());
}

void decodeModel(const json & j,
                 lt::Model & model) // [[expects: model.platform]]
{
    j.at("name").get_to(model.name);
    j.at("id").get_to(model.id);

    // Axes
    if (auto axes = j.find("axes"); axes != j.end())
        axes->get_to(model.axisOffsets);

    auto platform = model.platform();
    // Platform will always be valid

    // Table offsets
    if (auto tables = j.find("tables"); tables != j.end())
    {
        for (auto & [id, offset] : tables->items())
        {
            // Get platform table
            if (const TableDefinition * platformTable = platform->getTable(id);
                platformTable != nullptr)
            {
                // Copy table and set offset
                TableDefinition table(*platformTable);
                table.offset = offset;
                model.tables.emplace(id, std::move(table));
            }
            else
            {
                // TODO: Log error?
            }
        }
    }

    // Identifiers
    if (auto identifiers = j.find("identifiers"); identifiers != j.end())
    {
        for (const auto & identifier : *identifiers)
        {
            const auto offset = identifier.at("offset").get<std::size_t>();
            const auto & data = identifier.at("data").get<std::string>();
            model.identifiers.emplace_back(offset, data.begin(), data.end());
        }
    }

    // Checksums
    if (auto checksums = j.find("checksums"); checksums != j.end())
    {
        for (const auto & node : *checksums)
        {
            model.checksums.add(node.get<ChecksumPtr>());
        }
    }
}

PlatformPtr Platform::loadDirectory(const std::filesystem::path & base_path)
{
    // Load main.json
    PlatformPtr platform = load_main(base_path / "main.json");

    // Load models
    for (auto & entry : fs::directory_iterator(base_path))
    {
        const fs::path & path = entry.path();
        if (path.extension() != ".json" || path.filename() == "main.json" ||
            !entry.is_regular_file())
        {
            continue;
        }
        std::ifstream file(path);
        json j;
        file >> j;

        auto model = std::make_shared<Model>(platform);
        decodeModel(j, *model);
        platform->models.emplace_back(std::move(model));
    }
    return platform;
}

PlatformPtr Platforms::find(const std::string & id) const noexcept
{
    auto it = std::find_if(platforms_.begin(), platforms_.end(),
                           [&](const PlatformPtr & p) { return p->id == id; });
    if (it == platforms_.end())
    {
        // No platform was found
        return PlatformPtr();
    }
    return *it;
}

void Platforms::loadDirectory(const std::filesystem::path & path)
{
    for (auto & entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            // Convert to shared_ptr and store
            platforms_.emplace_back(Platform::loadDirectory(entry.path()));
        }
    }
}

PlatformPtr Platforms::first() const noexcept
{
    if (platforms_.empty())
        return PlatformPtr();
    return platforms_.front();
}

ModelPtr Platforms::find(const std::string & platformId,
                         const std::string & modelId) const noexcept
{
    PlatformPtr platform = find(platformId);
    if (!platform)
        return ModelPtr();

    return platform->findModel(modelId);
}

} // namespace lt
