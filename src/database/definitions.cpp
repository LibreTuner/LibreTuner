#include "definitions.h"

#include "lt/definition/model.h"
#include "yaml-cpp/yaml.h"

#include "logger.h"

#include <fstream>

namespace fs = std::filesystem;

lt::ChecksumPtr loadChecksum(const YAML::Node &checksum) {
    const auto &mode = checksum["mode"].as<std::string>();
    const auto offset = checksum["offset"].as<std::size_t>();
    const auto size = checksum["size"].as<std::size_t>();
    const auto target = checksum["target"].as<std::size_t>();

    lt::ChecksumPtr sum;
    if (mode == "basic") {
        sum = lt::ChecksumPtr(new lt::ChecksumBasic(offset, size, target));
    } else {
        throw std::runtime_error("invalid mode for checksum");
    }

    const auto &modify = checksum["modify"];
    if (modify) {
        for (const YAML::Node &node : modify) {
            sum->addModifiable(node["offset"].as<std::size_t>(),
                               node["size"].as<std::size_t>());
        }
    }

    return sum;
}

lt::ModelPtr loadModel(const lt::Platform &platform, const YAML::Node &file) {
    auto model = std::make_shared<lt::Model>(platform);

    model->name = file["name"].as<std::string>();
    model->id = file["id"].as<std::string>();

    // Load axes
    const auto &axes = file["axes"];
    std::for_each(axes.begin(), axes.end(), [&](const YAML::Node &axis) {
        const auto id = axis["id"].as<std::string>();
        const auto offset = axis["offset"].as<std::size_t>();

        model->axisOffsets.emplace(id, offset);
    });

    // Load table offsets
    const auto &tables = file["tables"];
    for (YAML::const_iterator it = tables.begin(); it != tables.end(); ++it) {
        std::size_t id = it->first.as<std::size_t>();
        const lt::TableDefinition *tableDef = platform.getTable(id);
        if (tableDef == nullptr) {
            throw std::runtime_error(
                "offset declared for nonexistant table with id " +
                std::to_string(id));
        }
        if (model->tables.size() <= id) {
            model->tables.resize(id + 1);
        }
        model->tables[id] =
            lt::ModelTable{tableDef, it->second.as<std::size_t>()};
    }

    // Load identifiers
    const auto &identifiers = file["identifiers"];
    std::for_each(
        identifiers.begin(), identifiers.end(),
        [&](const YAML::Node &identifier) {
            const auto offset = identifier["offset"].as<std::size_t>();
            const auto &data = identifier["data"].as<std::string>();

            model->identifiers.emplace_back(offset, data.begin(), data.end());
        });

    // Load checksums
    const auto &checksums = file["checksums"];
    std::for_each(checksums.begin(), checksums.end(),
                  [&](const YAML::Node &node) {
                      lt::ChecksumPtr checksum = loadChecksum(node);
                      model->checksums.add(std::move(checksum));
                  });

    return model;
}

lt::TableDefinition loadTable(std::size_t id, const YAML::Node &node) {
    lt::TableDefinition definition;
    definition.id = id;
    definition.name = node["name"].as<std::string>();
    definition.description = node["description"].as<std::string>();

    if (node["category"]) {
        definition.category = node["category"].as<std::string>();
    }

    const auto get_datatype = [&](const std::string &type) {
        if (type == "float") {
            return lt::DataType::Float;
        }
        if (type == "uint8") {
            return lt::DataType::Uint8;
        }
        if (type == "uint16") {
            return lt::DataType::Uint16;
        }
        if (type == "uint32") {
            return lt::DataType::Uint32;
        }
        if (type == "int8") {
            return lt::DataType::Int8;
        }
        if (type == "int16") {
            return lt::DataType::Int16;
        }
        if (type == "int32") {
            return lt::DataType::Int32;
        }

        throw std::runtime_error("invalid datatype");
    };

    definition.dataType = get_datatype(node["datatype"].as<std::string>());
    if (node["storeddatatype"]) {
        definition.storedDataType =
            get_datatype(node["storeddatatype"].as<std::string>());
    } else {
        definition.storedDataType = definition.dataType;
    }

    if (node["width"]) {
        definition.width = node["width"].as<std::size_t>();
    }
    if (node["height"]) {
        definition.height = node["height"].as<std::size_t>();
    }

    if (node["axisx"]) {
        definition.axisX = node["axisx"].as<std::string>();
    }
    if (node["axisy"]) {
        definition.axisY = node["axisy"].as<std::string>();
    }
    if (node["scale"]) {
        definition.scale = node["scale"].as<double>();
    }

    definition.minimum =
        node["minimum"].as<double>(std::numeric_limits<double>::min());
    definition.maximum =
        node["maximum"].as<double>(std::numeric_limits<double>::max());

    return definition;
}

lt::Pid loadPid(const YAML::Node &pid) {
    lt::Pid definition;
    definition.valid = true;
    definition.id = static_cast<uint32_t>(pid["id"].as<std::size_t>());

    definition.name = pid["name"].as<std::string>();
    definition.description = pid["description"].as<std::string>();
    definition.code = static_cast<uint16_t>(pid["code"].as<std::size_t>());
    definition.formula = pid["formula"].as<std::string>();
    definition.unit = pid["unit"].as<std::string>();

    return definition;
}

lt::AxisDefinition loadAxis(const YAML::Node &axis) {
    lt::AxisDefinition definition;

    definition.name = axis["name"].as<std::string>();
    definition.id = axis["id"].as<std::string>();
    definition.type = [&](const std::string &type) -> lt::AxisType {
        if (type == "linear") {
            return lt::AxisType::Linear;
        }
        if (type == "memory") {
            return lt::AxisType::Memory;
        }
        throw std::runtime_error("invalid axis type");
    }(axis["type"].as<std::string>());

    definition.dataType = [&](const std::string &type) {
        if (type == "float") {
            return lt::DataType::Float;
        }
        if (type == "uint8") {
            return lt::DataType::Uint8;
        }
        if (type == "uint16") {
            return lt::DataType::Uint16;
        }
        if (type == "uint32") {
            return lt::DataType::Uint32;
        }
        if (type == "int8") {
            return lt::DataType::Int8;
        }
        if (type == "int16") {
            return lt::DataType::Int16;
        }
        if (type == "int32") {
            return lt::DataType::Int32;
        }

        throw std::runtime_error("invalid datatype");
    }(axis["datatype"].as<std::string>());

    switch (definition.type) {
    case lt::AxisType::Linear:
        definition.start = axis["minimum"].as<double>();
        definition.increment = axis["increment"].as<double>();
        break;
    case lt::AxisType::Memory:
        definition.size = axis["size"].as<std::size_t>();
        break;
    }
    return definition;
}

lt::PlatformPtr loadPlatformDefinition(const std::filesystem::path &path) {
    auto platform = std::make_shared<lt::Platform>();

    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open " + path.string());
    }

    YAML::Node root = YAML::Load(file);

    platform->name = root["name"].as<std::string>();
    platform->id = root["id"].as<std::string>();
    platform->romsize = root["romsize"].as<std::size_t>();
    platform->baudrate = root["baudrate"].as<std::size_t>();
    if (root["logmode"]) {
        platform->logMode = root["logmode"].as<std::string>();
        std::transform(platform->logMode.begin(), platform->logMode.end(),
                       platform->logMode.begin(), ::tolower);
    }

    // Load transfer
    {
        const auto &transfer = root["transfer"];
        if (transfer["flashmode"]) {
            platform->flashMode = transfer["flashmode"].as<std::string>();
            std::transform(platform->flashMode.begin(), platform->flashMode.end(),
                           platform->flashMode.begin(), ::tolower);
        }

        if (transfer["downloadmode"]) {
            platform->downloadMode = transfer["downloadmode"].as<std::string>();
            std::transform(platform->downloadMode.begin(),
                           platform->downloadMode.end(),
                           platform->downloadMode.begin(), ::tolower);
        }

        platform->serverId = transfer["serverid"].as<std::size_t>();
    }

    // Load auth options
    if (root["auth"]) {
        const auto &auth = root["auth"];

        // Key should be the same for both
        if (auth["key"]) {
            platform->downloadAuthOptions.key = auth["key"].as<std::string>();
            platform->flashAuthOptions.key = platform->downloadAuthOptions.key;
        }

        // Session IDs
        if (auth["sessionid"]) {
            platform->downloadAuthOptions.session =
                auth["sessionid"].as<std::size_t>();
            platform->flashAuthOptions.session =
                platform->downloadAuthOptions.session;
        }
        if (auth["download_sessionid"]) {
            platform->downloadAuthOptions.session =
                auth["download_sessionid"].as<std::size_t>();
        }
        if (auth["flash_sessionid"]) {
            platform->flashAuthOptions.session =
                auth["flash_sessionid"].as<std::size_t>();
        }
    }

    // Load VIN patterns
    for (const auto &vin : root["vins"]) {
        platform->vins.emplace_back(vin.as<std::string>());
    }

    // Load axes
    for (const auto &node : root["axes"]) {
        lt::AxisDefinition axis = loadAxis(node);
        platform->axes.emplace(axis.id, std::move(axis));
    }

    // Load tables
    YAML::Node tables = root["tables"];
    for (auto it = tables.begin(); it != tables.end(); ++it) {
        std::size_t id = it->first.as<std::size_t>();
        lt::TableDefinition def = loadTable(id, it->second);
        if (id >= platform->tables.size()) {
            platform->tables.resize(id + 1);
        }
        platform->tables[id] = std::move(def);
    }

    for (const auto &pid : root["pids"]) {
        platform->pids.emplace_back(loadPid(pid));
    }

    return platform;
}

void Definitions::load() {
    platforms_.clear();

    if (!fs::is_directory(path_)) {
        return;
    }

    for (auto &sub : fs::directory_iterator(path_)) {
        if (sub.is_directory()) {
            loadPlatform(sub);
        }
    }
}

void Definitions::loadPlatform(const fs::path &path) {
    fs::path mainPath = path / "main.yaml";

    if (!fs::exists(mainPath)) {
        throw std::runtime_error(
            "main.yaml does not exist in platform directory " +
            mainPath.string());
    }

    if (!fs::is_regular_file(mainPath)) {
        throw std::runtime_error(
            "main.yaml exists but is not a file in platform directry " +
            mainPath.string());
    }

    lt::PlatformPtr platform = loadPlatformDefinition(mainPath);

    for (auto &modelEntry : fs::directory_iterator(path)) {
        if (modelEntry.path().filename() == "main.yaml") {
            continue;
        }
        if (modelEntry.is_regular_file()) {
            lt::ModelPtr model =
                loadModel(*platform, YAML::LoadFile(modelEntry.path().string()));
            platform->models.emplace_back(std::move(model));
        }
    }

    platforms_.emplace_back(std::move(platform));
}

lt::PlatformPtr Definitions::fromId(const std::string &id) const noexcept {
    for (const lt::PlatformPtr &platform : platforms_) {
        if (platform->id == id) {
            return platform;
        }
    }
    return lt::PlatformPtr();
}

lt::PlatformPtr Definitions::first() const noexcept
{
    if (platforms_.empty()) {
        return lt::PlatformPtr();
    }
    return platforms_[0];
}

QModelIndex Definitions::index(int row, int column,
                               const QModelIndex &parent) const {
    if (!parent.isValid()) {
        // Creating platform node
        return createIndex(row, column);
    }

    // Creating model node
    return createIndex(row, column, parent.row() + 1);
}

QModelIndex Definitions::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }

    if (child.internalId() == 0) {
        // Platform node
        return QModelIndex();
    } else {
        // Model node
        return createIndex(child.internalId() - 1, 0);
    }
}

int Definitions::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return platforms_.size();
    }

    if (parent.internalId() == 0) {
        // Platform node
        return platforms_[parent.row()]->models.size();
    }
    return 0;
}

int Definitions::columnCount(const QModelIndex &parent) const { return 1; }

QVariant Definitions::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.column() != 0 ||
        (role != Qt::DisplayRole && role != Qt::UserRole)) {
        return QVariant();
    }

    if (index.internalId() == 0) {
        // Platform
        std::size_t platformId = index.row();
        if (platformId >= platforms_.size()) {
            return QVariant();
        }
        const lt::PlatformPtr &platform = platforms_[platformId];
        if (role == Qt::DisplayRole) {
            return QString::fromStdString(platform->name);
        } else if (role == Qt::UserRole) {
            return QVariant::fromValue(platform);
        }
        return QVariant();
    }

    // Model
    std::size_t platformId = index.internalId() - 1;
    if (platformId >= platforms_.size()) {
        return QVariant();
    }
    const lt::PlatformPtr &platform = platforms_[platformId];

    std::size_t modelId = index.row();
    if (modelId >= platform->models.size()) {
        const lt::ModelPtr &model = platform->models[modelId];
        if (role == Qt::DisplayRole) {
            return QString::fromStdString(model->name);
        } else if (role == Qt::UserRole) {
            return QVariant::fromValue(model);
        }
    }

    return QVariant();
}

QVariant Definitions::headerData(int section, Qt::Orientation orientation,
                                 int role) const {
    return QVariant();
}
