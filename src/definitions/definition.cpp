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

#include "definition.h"
#include "piddefinitions.h"
#include "tabledefinitions.h"
#include "logger.h"

#include <QDir>

#include <algorithm>
#include <cassert>
#include <fstream>

namespace definition {

void Model::load(const YAML::Node& file) {
    name = file["name"].as<std::string>();
    id = file["id"].as<std::string>();
    
    // Load axes
    const auto &axes = file["axes"];
    std::for_each(axes.begin(), axes.end(), [&](const YAML::Node &axis) { loadAxis(axis); });
    
    // Load tables
    const auto &tables = file["tables"];
    for (YAML::const_iterator it = tables.begin(); it != tables.end(); ++it) {
        loadOffset(it->first.as<std::size_t>(), it->second.as<std::size_t>());
    }
    
    // Load identifiers
    const auto &identifiers = file["identifiers"];
    std::for_each(identifiers.begin(), identifiers.end(), [&](const YAML::Node &identifier) { loadIdentifier(identifier); });
    
    // Load checksums
    const auto &checksums = file["checksums"];
    std::for_each(checksums.begin(), checksums.end(), [&](const YAML::Node &checksum) { loadChecksum(checksum); });
    
}



void Model::loadOffset(std::size_t id, std::size_t offset) {
    if (tables.size() <= id) {
        tables.resize(id + 1);
    }
    tables[id] = offset;
}



void Model::loadAxis(const YAML::Node &axis) {
    const auto id = axis["id"].as<std::string>();
    const auto offset = axis["offset"].as<std::size_t>();
    
    axisOffsets.emplace(id, offset);
}



void Model::loadIdentifier(const YAML::Node &identifier) {
    const auto offset = identifier["offset"].as<std::size_t>();
    const auto &data = identifier["data"].as<std::string>();
    
    identifiers.emplace_back(offset, data.begin(), data.end());
}



void Model::loadChecksum(const YAML::Node &checksum) {
    const auto &mode = checksum["mode"].as<std::string>();
    const auto offset = checksum["offset"].as<std::size_t>();
    const auto size = checksum["size"].as<std::size_t>();
    const auto target =  checksum["target"].as<std::size_t>();
    
    Checksum *sum;
    if (mode == "basic") {
        sum = checksums.addBasic(offset, size, target);
    } else {
        throw std::runtime_error("invalid mode for checksum");
    }
    
    const auto &modify = checksum["modify"];
    if (modify) {
        for (auto it = modify.begin(); it != modify.end(); it++) {
            sum->addModifiable((*it)["offset"].as<std::size_t>(), (*it)["size"].as<std::size_t>());
        }
    }
}



std::size_t Model::getAxisOffset(const std::string& id)
{
    auto it = axisOffsets.find(id);
    if (it == axisOffsets.end()) {
        Logger::warning("No defined offset for axis '" + id + "'");
        return 0;
    }
    
    return it->second;
}



Model::Model(const Main& m) : main(m)
{
}



void Main::load(const std::string& dirPath)
{
    QDir dir(QString::fromStdString(dirPath));

    if (QFile::exists(QString::fromStdString(dirPath + "/main.yaml"))) {
        std::ifstream file(dirPath + "/main.yaml");
        if (!file.good()) {
            throw std::runtime_error("failed to open " + dirPath + "/main.yaml");
        }
        load(YAML::Load(file));
    } else {
        throw std::runtime_error(std::string("No main.yaml file in ") + dirPath);
    }

    for (QFileInfo &info :
        dir.entryInfoList(QDir::Filters(QDir::NoDotAndDotDot) | QDir::Files, QDir::NoSort)) {
        if (info.isFile()) {
            if (info.fileName().toLower() != "main.yaml") {
                // Model
                ModelPtr model = std::make_shared<Model>(*this);
                std::ifstream file(info.filePath().toStdString());
                if (!file.good()) {
                    throw std::runtime_error("failed to open " + dirPath + "/main.yaml");
                }
                model->load(YAML::Load(file));
                models.emplace_back(std::move(model));
            }
        }
    }
}



void Main::load(const YAML::Node& file)
{
    name = file["name"].as<std::string>();
    id = file["id"].as<std::string>();
    romsize = file["romsize"].as<std::size_t>();
    baudrate = file["baudrate"].as<std::size_t>();
    if (file["logmode"]) {
        logMode = [](std::string mode) {
            std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
            if (mode == "uds") {
                return DataLogMode::Uds;
            }
            return DataLogMode::None;
        }(file["logmode"].as<std::string>());
    }
    
    // Load transfer
    {
        const auto &transfer = file["transfer"];
        flashMode = [&](std::string mode) {
            std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
            if (mode == "mazdat1") {
                return FlashMode::T1;
            }
            return FlashMode::None;
        }(transfer["flashmode"].as<std::string>());
        
        downloadMode = [&](std::string mode) {
            std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
            if (mode == "mazda23") {
                return DownloadMode::Mazda23;
            }
            return DownloadMode::None;
        }(transfer["downloadmode"].as<std::string>());
        
        serverId = transfer["serverid"].as<std::size_t>();
    }
    
    Logger::debug("Loading auth options");
    // Load auth options
    if (file["auth"]) {
        const auto &auth = file["auth"];
        
        // Key should be the same for both
        if (auth["key"]) {
            downloadAuthOptions.key = auth["key"].as<std::string>();
            flashAuthOptions.key = downloadAuthOptions.key;
        }
        
        // Session IDs
        if (auth["sessionid"]) {
            downloadAuthOptions.session = auth["sessionid"].as<std::size_t>();
            flashAuthOptions.session = downloadAuthOptions.session;
        }
        if (auth["download_sessionid"]) {
            downloadAuthOptions.session = auth["download_sessionid"].as<std::size_t>();
        }
        if (auth["flash_sessionid"]) {
            flashAuthOptions.session = auth["flash_sessionid"].as<std::size_t>();
        }
    }
    
    // Load VIN patterns
    for (const auto &vin : file["vins"]) {
        vins.emplace_back(vin.as<std::string>());
    }
    
    // Load axes
    for (const auto &axis : file["axes"]) {
        loadAxis(axis);
    }
    
    // Load tables
    YAML::Node tables = file["tables"];
    for (auto it = tables.begin(); it != tables.end(); ++it) {
        loadTable(it->first.as<std::size_t>(), it->second);
    }
    
    
    for (const auto &pid : file["pids"]) {
        loadPid(pid);
    }
}



void Main::loadPid(const YAML::Node& pid)
{
    Pid definition;
    definition.valid = true;
    definition.id = static_cast<uint32_t>(pid["id"].as<std::size_t>());
    
    definition.name = pid["name"].as<std::string>();
    definition.description = pid["description"].as<std::string>();
    definition.code = static_cast<uint16_t>(pid["code"].as<std::size_t>());
    definition.formula = pid["formula"].as<std::string>();
    definition.unit = pid["unit"].as<std::string>();
    
    pids.emplace_back(std::move(definition));
}



void Main::loadTable(std::size_t id, const YAML::Node& table)
{
    Table definition;
    definition.id = id;
    definition.name = table["name"].as<std::string>();
    definition.description = table["description"].as<std::string>();
    
    if (table["category"]) {
        definition.category = table["category"].as<std::string>();
    }
    
    const auto get_datatype = [&](const std::string &type) {
        if (type == "float") {
            return TableType::Float;
        }
        if (type == "uint8") {
            return TableType::Uint8;
        }
        if (type == "uint16") {
            return TableType::Uint16;
        }
        if (type == "uint32") {
            return TableType::Uint32;
        }
        if (type == "int8") {
            return TableType::Int8;
        }
        if (type == "int16") {
            return TableType::Int16;
        }
        if (type == "int32") {
            return TableType::Int32;
        }
        
        throw std::runtime_error("invalid datatype");
    };
    
    definition.dataType = get_datatype(table["datatype"].as<std::string>());
    if (table["storeddatatype"]) {
        definition.storedDataType = get_datatype(table["storeddatatype"].as<std::string>());
    } else {
        definition.storedDataType = definition.dataType;
    }
    
    if (table["width"]) {
        definition.sizeX = table["width"].as<std::size_t>();
    }
    if (table["height"]) {
        definition.sizeY = table["height"].as<std::size_t>();
    }
    
    if (table["axisx"]) {
        definition.axisX = table["axisx"].as<std::string>();
    }
    if (table["axisy"]) {
        definition.axisY = table["axisy"].as<std::string>();
    }
    if (table["scale"]) {
        definition.scale = table["scale"].as<double>();
    }
    
    const auto opt_double = [&](const YAML::Node &node, double def) {
        if (node) {
            return node.as<double>();
        }
        return def;
    };
    
    definition.minimum = opt_double(table["minimum"], std::numeric_limits<double>::min());
    definition.maximum = opt_double(table["maximum"], std::numeric_limits<double>::max());
    
    if (definition.id >= tables.size()) {
        tables.resize(id + 1);
    }
    
    tables[id] = std::move(definition);
}



void Main::loadAxis(const YAML::Node& axis)
{
    Axis definition;
    
    definition.name = axis["name"].as<std::string>();
    definition.id = axis["id"].as<std::string>();
    definition.type = [&] (const std::string &type) -> AxisType {
        if (type == "linear") {
            return AxisType::Linear;
        }
        if (type == "memory") {
            return AxisType::Memory;
        }
        throw std::runtime_error("invalid axis type");
    }(axis["type"].as<std::string>());
    
    definition.dataType = [&](const std::string &type) {
        if (type == "float") {
            return TableType::Float;
        }
        if (type == "uint8") {
            return TableType::Uint8;
        }
        if (type == "uint16") {
            return TableType::Uint16;
        }
        if (type == "uint32") {
            return TableType::Uint32;
        }
        if (type == "int8") {
            return TableType::Int8;
        }
        if (type == "int16") {
            return TableType::Int16;
        }
        if (type == "int32") {
            return TableType::Int32;
        }
        
        throw std::runtime_error("invalid datatype");
    }(axis["datatype"].as<std::string>());

    switch (definition.type) {
    case AxisType::Linear:
        definition.start = axis["minimum"].as<double>();
        definition.increment = axis["increment"].as<double>();
        break;
    case AxisType::Memory:
        definition.size = axis["size"].as<std::size_t>();
        break;
    }

    axes.emplace(definition.id, std::move(definition));
}



definition::ModelPtr Main::identify(const uint8_t *data, size_t size)
{
    for (const definition::ModelPtr &def : models) {
        if (checkModel(*def, data, size)) {
            return def;
        }
    }
    return nullptr;
}



definition::ModelPtr Main::findModel(const std::string& id)
{
    for (const definition::ModelPtr &model : models) {
        if (model->id == id) {
            return model;
        }
    }
    return nullptr;
}



bool Main::matchVin(const std::string& vin)
{
    return std::any_of(vins.begin(), vins.end(), [&vin](const auto &pattern) {
        return std::regex_match(vin, pattern);
    });
}


Pid *Main::getPid(uint32_t id) {
    auto it = std::find_if(pids.begin(), pids.end(), [id](Pid &pid) { return pid.id == id; });
    if (it != pids.end()) {
        return &*it;
    }
    return nullptr;
}


bool checkModel(const definition::Model& model, const uint8_t *data, size_t size)
{
    for (const Identifier &identifier : model.identifiers) {
        if (identifier.offset() + identifier.size() > size) {
            return false;
        }

        if (!std::equal(data + identifier.offset(), data + identifier.offset() + identifier.size(),
                       identifier.data())) {
            return false;
        }
    }
    return true;
}



std::size_t Table::rawSize() const
{
    return tableTypeSize(storedDataType) * sizeX * sizeY;
}



std::size_t Model::getOffset(std::size_t index)
{
    if (index >= tables.size()) {
        return 0;
    }
    
    return tables[index];
}

}
