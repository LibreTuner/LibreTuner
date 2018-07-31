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

class Definition;

/* Subtype definition. Includes the table locations */
// Subtype is a misnomer, but it's too late to change (or is it?)
class SubDefinition {
public:
  explicit SubDefinition(Definition *definition);

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
  std::string lastError_;

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
typedef std::shared_ptr<SubDefinition> SubDefinitionPtr;
typedef std::weak_ptr<SubDefinition> SubDefinitionWeakPtr;

enum class LogMode {
  None,
  Uds,
};

/**
 * An ECU definition
 */
class Definition {
public:
  /* Loads a definition directory */
  bool load(const QString &path);

  /* Loads the main definition file into definition. */
  bool loadMain(const QString &path);

  /* Loads a subtype (ECU model or firmware version) definition. */
  bool loadSubtype(const QString &path);

  /* Attempts to determine the subtype of the data. Returns
   * nullptr if no subtypes match. */
  SubDefinitionPtr identifySubtype(gsl::span<const uint8_t> data);

  /* Returns the axis with the specified ID. If none exists, returns
   * nullptr. */
  TableAxis *getAxis(const std::string &id);

  std::string name() const { return name_; }

  std::string id() const { return id_; }

  std::string lastError() const { return lastError_; }

  uint32_t size() const { return size_; }

  const TableDefinitions *tables() const { return &tables_; }

  TableDefinitions *tables() { return &tables_; }

  PidDefinitions &pids() { return pids_; }

  Endianness endianness() const { return endianness_; }

  DownloadMode downloadMode() const { return downloadMode_; }

  LogMode logMode() const { return logMode_; }

  FlashMode flashMode() const { return flashMode_; }

  std::string key() const { return key_; }

  unsigned serverId() const { return serverId_; }

  uint32_t baudrate() const { return baudrate_; }

  size_t flashOffset() const { return flashOffset_; }

  size_t flashSize() const { return flashSize_; }

  /* Returns the subtype definition with the id. Returns blank pointer
   * if the subtype does not exist. */
  SubDefinitionPtr findSubtype(const std::string &id);

  /* Gets the axis id from the string identifier. Returns -1
   * if the id does not exist. */
  int axisId(const std::string &id);

  /* Returns true if the supplied VIN matches the definition */
  bool matchVin(const std::string &vin);

private:
  std::string lastError_;

  std::string name_;
  std::string id_;

  DownloadMode downloadMode_;
  FlashMode flashMode_;
  uint32_t baudrate_ = 500000;
  LogMode logMode_ = LogMode::None;
  /* Security key */
  std::string key_;
  /* Server ID for ISO-TP reqeusts */
  unsigned serverId_;

  /* Flash region */
  size_t flashOffset_, flashSize_;

  Endianness endianness_;

  int lastAxisId_ = 0;
  uint32_t size_;

  TableDefinitions tables_;
  PidDefinitions pids_;
  std::unordered_map<std::string, TableAxisPtr> axes_;
  std::vector<SubDefinitionPtr> subtypes_;
  std::vector<std::regex> vins_;

  void readTables(QXmlStreamReader &xml);
  void loadAxes(QXmlStreamReader &xml);
  void loadVins(QXmlStreamReader &xml);
  void loadPids(QXmlStreamReader &xml);
};
typedef std::shared_ptr<Definition> DefinitionPtr;

#endif // DEFINITION_H
