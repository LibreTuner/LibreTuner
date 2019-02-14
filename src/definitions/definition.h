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

#include <regex>
#include <string>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

#include "checksummanager.h"
#include "endian.h"
#include "enums.hpp"
#include "udsauthenticator.h"


namespace definition {
    
enum class TableCategory {
    Limiter, // Any type of limiter (RPM limiter, speed limiter)
    Miscellaneous,
};


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


enum class AxisType {
    Linear,
    Memory,
};

struct Axis {
    std::string name;
    std::string id;
    AxisType type;
    TableType dataType;
    union {
        struct {
            double start;
            double increment;
        };
        struct {
            std::size_t size;
        };
    };
};



struct Table {
    int id = -1;
    std::string name;
    std::string description;
    std::string category;
    TableType dataType;
    TableType storedDataType;
    std::size_t sizeX{1};
    std::size_t sizeY{1};
    double maximum;
    double minimum;
    double scale = 1.0;
    std::string axisX;
    std::string axisY;
    
    /* Returns the raw size in bytes */
    std::size_t rawSize() const;
};



struct Pid {
    std::string name;
    std::string description;
    std::string formula;
    std::string unit;
    uint32_t id;
    uint16_t code;

    bool valid = false;
};



/* Model definition. Includes the table locations */
struct Model {
public:
    explicit Model(const Main &main);
    
    const Main &main;
    std::string id;
    std::string name;
    ChecksumManager checksums;
    
    /* Table offsets */
    std::vector<std::size_t> tables;
    
    std::unordered_map<std::string, std::size_t> axisOffsets;
    std::vector<Identifier> identifiers;
    
    std::size_t getOffset(std::size_t index);
    
    void load(const YAML::Node &file);
    void loadOffset(std::size_t id, std::size_t offset);
    void loadAxis(const YAML::Node &axis);
    void loadIdentifier(const YAML::Node &identifier);
    void loadChecksum(const YAML::Node &checksum);
    
    std::size_t getAxisOffset(const std::string &id);
};
using ModelPtr = std::shared_ptr<Model>;

bool checkModel(const Model &model, const uint8_t *data, size_t size);

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
    
    auth::Options downloadAuthOptions;
    auth::Options flashAuthOptions;
    
    /* Server ID for ISO-TP reqeusts */
    unsigned serverId {0x7e0};

    /* Flash region */
    size_t flashOffset, flashSize;

    Endianness endianness;

    int lastAxisId = 0;
    uint32_t romsize;

    std::vector<Table> tables;
    std::vector<Pid> pids;
    // axes MUST NOT be added to after initialization
    std::unordered_map<std::string, Axis> axes;
    std::vector<ModelPtr> models;
    std::vector<std::regex> vins;
    
    void load(const std::string &dirPath);
    
    void load(const YAML::Node &file);
    void loadTable(std::size_t id, const YAML::Node &table);
    void loadAxis(const YAML::Node &axis);
    void loadPid(const YAML::Node &pid);
    
    /* Returns true if the supplied VIN matches any pattern in vins */
    bool matchVin(const std::string &vin);
    
    /* Searched for a model definition from an id. Returns nullptr
    * if the id does not match a model. */
    ModelPtr findModel(const std::string &id);
    
    /* Attempts to determine the model of the data. Returns
    * nullptr if no models match. */
    ModelPtr identify(const uint8_t *data, size_t size);

    // Returns the PID with id `id` or nullptr if none exist
    Pid *getPid(uint32_t id);
};


typedef std::shared_ptr<Main> MainPtr;
}

#endif // DEFINITION_H
