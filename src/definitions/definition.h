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
#include <QXmlStreamReader>

#include "checksummanager.h"
#include "downloadinterface.h"
#include "endian.h"
#include "flasher.h"
#include "piddefinitions.h"
#include "tabledefinitions.h"

namespace definition {

struct Main;

/* Subtype definition. Includes the table locations */
// Subtype is a misnomer, but it's too late to change (or is it?)
class Sub {
public:
    explicit Sub(Definition *definition);

    /* Attempts to load a subtype definition. Returns false and sets
     * lastError on failure. */
    bool load(const QString &path);

    /* Returns the location of the table
     * If ok is not nullptr, sets to true if the table
     * exists and false if it does not exist. Returns 0 on failure,
     * however, ok should be used for error handling. */
    uint32_t getTableLocation(int tableId, bool *ok = nullptr);

    /* Returns the location of the axis. axisId can be obtained from
     * Definition::axisId(string_id). If ok is not nullptr, sets to
     * true if the axis exists and false if it does not exist.
     * Returns 0 on failure, however, ok should be used for
     * error handling. */
    uint32_t getAxisLocation(int axisId, bool *ok = nullptr);

    /* Returns true if the ROM data is of the subtype/firmware version */
    bool check(gsl::span<const uint8_t> data);

    Definition *definition() const { return definition_; }

    std::string lastError() const { return lastError_; }

    std::string id() const { return id_; }

    const ChecksumManager *checksums() const { return &checksums_; }

    ChecksumManager *checksums() { return &checksums_; }

    class Identifier {
    public:
        Identifier(uint32_t offset, const uint8_t *data, size_t length)
            : offset_(offset), data_(data, data + length) {}

        uint32_t offset() const { return offset_; }

        const uint8_t *data() const { return data_.data(); }

        size_t size() const { return data_.size(); }

    private:
        uint32_t offset_;
        std::vector<uint8_t> data_;
    };

private:
    Definition *definition_;
    std::string id_;
    std::string name_;

    ChecksumManager checksums_;

    /* Table offsets */
    std::vector<uint32_t> locations_;

    std::vector<uint32_t> axesOffsets_;

    std::vector<Identifier> identifiers_;

    /* Loads table locations from the current element. */
    void loadTables(QXmlStreamReader &xml);

    /* Loads model-specific axis information from current element */
    void loadAxes(QXmlStreamReader &xml);

    void loadChecksums(QXmlStreamReader &xml);

    void loadIdentifiers(QXmlStreamReader &xml);

    // Assign an id (automatically) to each axis for easy lookup?
};
using SubPtr = std::shared_ptr<Sub>;

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
    uint32_t size;

    TableDefinitions tables;
    PidDefinitions pids;
    std::unordered_map<std::string, TableAxisPtr> axes;
    std::vector<SubDefinitionPtr> subtypes;
    std::vector<std::regex> vins;
};

/* Returns true if the supplied VIN matches any pattern in vins */
bool match_vin(const std::vector<std::regex> &vins, const std::string &vin);

/* Searched for a subtype definition from an id. Returns nullptr
 * if the id does not match a subtype. */
const SubDefinitionPtr &find_subtype(const std::vector<SubPtr> &subtypes, const std::string &id);

/* Attempts to determine the subtype of the data. Returns
 * nullptr if no subtypes match. */
const SubDefinitionPtr &identify(const std::vector<SubPtr> &subtypes, gsl::span<const uint8_t> data);

typedef std::shared_ptr<Main> MainPtr;
}

#endif // DEFINITION_H
