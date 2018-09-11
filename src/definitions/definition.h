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

#ifndef DEFINITION_H
#define DEFINITION_H

#include <gsl/span>
#include <regex>
#include <string>
#include <unordered_map>

#include <QString>

#include <toml/toml.hpp>

#include "checksummanager.h"
#include "downloadinterface.h"
#include "endian.h"
#include "flasher.h"
#include "piddefinitions.h"
#include "tabledefinitions.h"

namespace definition {

struct Main;

/* Used to identify subtypes */
class Identifier {
public:
    template<class InputIt>
    Identifier(uint32_t offset, InputIt first, InputIt end)
        : offset_(offset), data_(first, end) {}

    uint32_t offset() const { return offset_; }

    const uint8_t *data() const { return data_.data(); }

    size_t size() const { return data_.size(); }

private:
    uint32_t offset_;
    std::vector<uint8_t> data_;
};


/* Model definition. Includes the table locations */
struct Model {
public:
    Model(const Main &main);
    
    const Main &main_;
    std::string id_;
    std::string name_;
    ChecksumManager checksums_;
    
    /* Table offsets */
    std::vector<std::size_t> tables_;
    
    std::unordered_map<std::string, std::size_t> axisOffsets_;
    std::vector<Identifier> identifiers_;
    
    
    void load(const toml::table &file);
    void loadTable(const toml::table &table);
    void loadAxis(const toml::table &axis);
    void loadIdentifier(const toml::table &identifier);
    void loadChecksum(const toml::table &checksum);
};
using ModelPtr = std::shared_ptr<Model>;

enum class LogMode {
    None,
    Uds,
};

/**
 * An ECU definition
 */
struct Main {
    std::string name;
    std::string id;

    DownloadMode downloadMode;
    FlashMode flashMode;
    uint32_t baudrate {500000};
    LogMode logMode = LogMode::None;
    /* Security key */
    std::string key;
    /* Server ID for ISO-TP reqeusts */
    unsigned serverId {0x7e0};

    /* Flash region */
    size_t flashOffset, flashSize;

    Endianness endianness;

    int lastAxisId = 0;
    uint32_t romsize;

    TableDefinitions tables;
    PidDefinitions pids;
    std::unordered_map<std::string, TableAxisPtr> axes;
    std::vector<ModelPtr> models;
    std::vector<std::regex> vins;
    
    
    void load(const toml::table &file);
    void loadTable(const toml::table &table);
};

/* Returns true if the supplied VIN matches any pattern in vins */
bool match_vin(const std::vector<std::regex> &vins, const std::string &vin);

/* Searched for a model definition from an id. Returns nullptr
 * if the id does not match a model. */
const ModelPtr &find_subtype(const std::vector<ModelPtr> &models, const std::string &id);

/* Attempts to determine the model of the data. Returns
 * nullptr if no models match. */
const ModelPtr &identify(const std::vector<ModelPtr> &models, gsl::span<const uint8_t> data);

typedef std::shared_ptr<Main> MainPtr;
}

#endif // DEFINITION_H
