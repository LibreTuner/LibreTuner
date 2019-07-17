#include "platform.h"
#include "../support/util.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

namespace lt
{

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

const Pid * Platform::getPid(uint32_t id) const noexcept
{
    for (const Pid & pid : pids)
    {
        if (pid.code == id)
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

    YAML::Node root = YAML::Load(file);
    return std::make_shared<Platform>(root.as<Platform>());
}

ChecksumPtr loadChecksum(const YAML::Node & checksum)
{
    const auto & mode = checksum["mode"].as<std::string>();
    const auto offset = checksum["offset"].as<std::size_t>();
    const auto size = checksum["size"].as<std::size_t>();
    const auto target = checksum["target"].as<std::size_t>();

    ChecksumPtr sum;
    if (mode == "basic")
        sum = lt::ChecksumPtr(new lt::ChecksumBasic(offset, size, target));
    else
        throw std::runtime_error("invalid mode for checksum");

    const auto & modify = checksum["modify"];
    if (modify)
    {
        for (const YAML::Node & node : modify)
        {
            sum->addModifiable(node["offset"].as<std::size_t>(),
                               node["size"].as<std::size_t>());
        }
    }

    return sum;
}

void decodeModel(const YAML::Node & node,
                 lt::Model & model) // [[expects: model.platform]]
{
    model.name = node["name"].as<std::string>();
    model.id = node["id"].as<std::string>();

    // Axes
    if (const auto & axes = node["axes"])
    {
        for (const auto & axis : axes)
        {
            const auto id = axis["id"].as<std::string>();
            const auto offset = axis["offset"].as<std::size_t>();

            model.axisOffsets.emplace(id, offset);
        }
    }

    auto platform = model.platform();
    // Platform will always be valid

    // Table offsets
    if (const auto & tables = node["tables"])
    {
        for (auto it = tables.begin(); it != tables.end(); ++it)
        {
            auto id = it->first.as<std::string>();
            // Get platform table
            if (const TableDefinition * platformTable = platform->getTable(id);
                platformTable != nullptr)
            {
                // Copy table and set offset
                TableDefinition table(*platformTable);
                table.offset = it->second.as<int>();
                model.tables.emplace(id, std::move(table));
            }
            else
            {
                // TODO: Log error?
            }
        }
    }

    // Identifiers
    if (const auto & identifiers = node["identifiers"])
    {
        for (const auto & identifier : identifiers)
        {
            const auto offset = identifier["offset"].as<std::size_t>();
            const auto & data = identifier["data"].as<std::string>();

            model.identifiers.emplace_back(offset, data.begin(), data.end());
        }
    }

    // Checksums
    if (const auto & checksums = node["checksums"])
    {
        for (const auto & node : checksums)
        {
            model.checksums.add(lt::loadChecksum(node));
        }
    }
}

PlatformPtr Platform::loadDirectory(const std::filesystem::path & base_path)
{
    // Load main.yaml
    PlatformPtr platform = load_main(base_path / "main.yaml");

    // Load models
    for (auto & entry : fs::directory_iterator(base_path))
    {
        const fs::path & path = entry.path();
        if (path.extension() != ".yaml" || path.filename() == "main.yaml" ||
            !entry.is_regular_file())
        {
            continue;
        }
        std::ifstream file(path);

        auto model = std::make_shared<Model>(platform);
        decodeModel(YAML::Load(file), *model);
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

// Declare YAML conversions outside lt namespace
namespace YAML
{
template <> struct convert<lt::Pid>
{
    static bool decode(const Node & node, lt::Pid & pid)
    {
        pid.name = node["name"].as<std::string>();
        pid.description = node["description"].as<std::string>();
        pid.code = static_cast<uint16_t>(node["code"].as<std::size_t>());
        pid.formula = node["formula"].as<std::string>();
        pid.unit = node["unit"].as<std::string>();
        return true;
    }
};

template <> struct convert<lt::TableDefinition>
{
    static bool decode(const Node & node, lt::TableDefinition & table)
    {
        table.name = node["name"].as<std::string>();
        table.description = node["description"].as<std::string>();

        if (node["category"])
            table.category = node["category"].as<std::string>();

        table.dataType =
            lt::datatype_from_string(node["datatype"].as<std::string>());
        if (node["storeddatatype"])
            table.storedDataType = lt::datatype_from_string(
                node["storeddatatype"].as<std::string>());
        else
            table.storedDataType = table.dataType;

        if (const auto & n = node["width"])
            table.width = n.as<std::size_t>();
        if (const auto & n = node["height"])
            table.height = n.as<std::size_t>();

        if (const auto & n = node["axisx"])
            table.axisX = n.as<std::string>();
        if (const auto & n = node["axisy"])
            table.axisY = n.as<std::string>();
        if (const auto & n = node["scale"])
            table.scale = n.as<double>();

        table.minimum =
            node["minimum"].as<double>(std::numeric_limits<double>::min());
        table.maximum =
            node["maximum"].as<double>(std::numeric_limits<double>::max());
        return true;
    }
};

template <> struct convert<lt::AxisDefinition>
{
    static bool decode(const Node & node, lt::AxisDefinition & axis)
    {
        axis.name = node["name"].as<std::string>();
        axis.dataType =
            lt::datatype_from_string(node["datatype"].as<std::string>());
        std::string type = node["type"].as<std::string>();
        if (type == "memory")
        {
            lt::MemoryAxisDefinition memory;
            memory.size = node["size"].as<std::size_t>();
            axis.def.emplace<lt::MemoryAxisDefinition>(std::move(memory));
        }
        else if (type == "linear")
        {
            lt::LinearAxisDefinition linear;
            linear.start = node["minimum"].as<double>();
            linear.increment =
                node["increment"].as<double>(); // TODO: rename to "step"
            linear.size = node["size"].as<int>();
            axis.def.emplace<lt::LinearAxisDefinition>(std::move(linear));
        }
        else
        {
            throw std::runtime_error("invalid axis type '" + type + "'");
        }
        return true;
    }
};

template <> struct convert<lt::Platform>
{
    static bool decode(const Node & node, lt::Platform & platform)
    {
        platform.id = node["id"].as<std::string>();
        platform.name = node["name"].as<std::string>();
        platform.romsize = node["romsize"].as<std::size_t>();
        platform.baudrate = node["baudrate"].as<std::size_t>();
        if (const auto & n = node["logmode"])
        {
            platform.logMode = n.as<std::string>();
            lt::lowercase_string(platform.logMode);
        }

        if (const auto & n = node["endianness"])
        {
            std::string endianness = n.as<std::string>();
            lt::lowercase_string(endianness);
            if (endianness == "big")
                platform.endianness = lt::Endianness::Big;
            else if (endianness == "litte")
                platform.endianness = lt::Endianness::Little;
            else
                throw std::runtime_error("invalid endianness value '" + endianness + "'");
        }

        // Transfer
        if (const auto & transfer = node["transfer"])
        {
            if (const auto & n = transfer["flashmode"])
            {
                platform.flashMode = n.as<std::string>();
                lt::lowercase_string(platform.flashMode);
            }
            if (const auto & n = transfer["downloadmode"])
            {
                platform.downloadMode = n.as<std::string>();
                lt::lowercase_string(platform.downloadMode);
            }
            platform.serverId = transfer["serverid"].as<std::size_t>();
        }

        // Authentication
        if (const auto & auth = node["auth"])
        {
            // Key should be the same for both
            if (const auto & n = auth["key"])
            {
                platform.downloadAuthOptions.key = n.as<std::string>();
                platform.flashAuthOptions.key =
                    platform.downloadAuthOptions.key;
            }

            // Session IDs
            if (const auto & n = auth["sessionid"])
            {
                platform.downloadAuthOptions.session =
                    static_cast<uint8_t>(n.as<std::size_t>());
                platform.flashAuthOptions.session =
                    platform.downloadAuthOptions.session;
            }
            if (const auto & n = auth["download_sessionid"])
            {
                platform.downloadAuthOptions.session =
                    static_cast<uint8_t>(n.as<std::size_t>());
            }
            if (const auto & n = auth["flash_sessionid"])
            {
                platform.flashAuthOptions.session =
                    static_cast<uint8_t>(n.as<std::size_t>());
            }
        }

        // VIN patterns
        for (const auto & vin : node["vins"])
        {
            platform.vins.emplace_back(vin.as<std::string>());
        }

        if (const auto & axes = node["axes"])
        {
            for (auto it = axes.begin(); it != axes.end(); ++it)
            {
                platform.axes.emplace(it->first.as<std::string>(),
                                      it->second.as<lt::AxisDefinition>());
            }
        }

        // Tables
        if (const auto & tables = node["tables"])
        {
            for (auto it = tables.begin(); it != tables.end(); ++it)
            {
                auto id = it->first.as<std::string>();
                auto def = it->second.as<lt::TableDefinition>();
                def.id = id;
                platform.tables.emplace(id, std::move(def));
            }
        }

        if (const auto & pids = node["pids"])
            platform.pids = pids.as<std::vector<lt::Pid>>();
        return true;
    }
};
} // namespace YAML
