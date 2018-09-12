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
#include "endian.h"
#include "enums.hpp"

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



class AxisData {
public:
    virtual ~AxisData() = default;
    
    /* Returns the label for the given axis position. */
    virtual double label(std::size_t idx) const =0;
};



class LinearAxis : public AxisData {
public:
    LinearAxis(double start, double increment);
    
    virtual double label(std::size_t idx) const override;
    
private:
    double start_;
    double increment_;
};



struct Axis {
    std::string name;
    std::string id;
    
    std::unique_ptr<AxisData> data;
};



struct Table {
    int id;
    std::string name;
    std::string description;
    TableType type;
    TableCategory category;
    TableType dataType;
    std::size_t sizeX;
    std::size_t sizeY;
    double maximum;
    double minimum;
    std::string axisXId;
    std::string axisYId;
    
    /* Returns the raw size in bytes */
    std::size_t rawSize() const;
};



/* Model definition. Includes the table locations */
struct Model {
public:
    Model(const Main &main);
    
    const Main &main;
    std::string id;
    std::string name;
    ChecksumManager checksums;
    
    /* Table offsets */
    std::vector<std::size_t> tables;
    
    std::unordered_map<std::string, std::size_t> axisOffsets;
    std::vector<Identifier> identifiers;
    
    
    void load(const toml::table &file);
    void loadTable(const toml::table &table);
    void loadAxis(const toml::table &axis);
    void loadIdentifier(const toml::table &identifier);
    void loadChecksum(const toml::table &checksum);
};
using ModelPtr = std::shared_ptr<Model>;

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

    std::vector<Table> tables;
    //PidDefinitions pids;
    std::unordered_map<std::string, std::unique_ptr<Axis>> axes;
    std::vector<ModelPtr> models;
    std::vector<std::regex> vins;
    
    void load(const std::string &dirPath);
    
    void load(const toml::table &file);
    void loadTable(const toml::table &table);
    void loadAxis(const toml::table &axis);
    
    /* Returns true if the supplied VIN matches any pattern in vins */
    bool matchVin(const std::string &vin);
    
    /* Searched for a model definition from an id. Returns nullptr
    * if the id does not match a model. */
    const ModelPtr &findModel(const std::string &id);
    
    /* Attempts to determine the model of the data. Returns
    * nullptr if no models match. */
    const ModelPtr &identify(gsl::span<const uint8_t> data);
};


typedef std::shared_ptr<Main> MainPtr;
}

#endif // DEFINITION_H
