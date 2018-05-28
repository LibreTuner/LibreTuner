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

#ifndef TABLEDEFINITIONS_H
#define TABLEDEFINITIONS_H

#include <limits>
#include <memory>
#include <string>

#include <QXmlStreamReader>

#define MAX_TABLEID 1000

enum TableType {
  TABLE_1D,
  TABLE_2D,
  TABLE_3D, // Do ECUs ever use these?
};

enum TableCategory {
  TCAT_LIMITER, // Any type of limiter (RPM limiter, speed limiter)
  TCAT_MISC,
};

enum DataType {
  TDATA_UINT8,
  TDATA_UINT16,
  TDATA_UINT32,
  TDATA_FLOAT,
  TDATA_INT8,
  TDATA_INT16,
  TDATA_INT32,
};

class Definition;

class TableAxis;
typedef std::shared_ptr<TableAxis> TableAxisPtr;

class TableAxis {
public:
  /* Attempts to load a table axis from an xml element.
   * Returns nullptr on failure and raises an xml error. */
  static TableAxisPtr load(QXmlStreamReader &xml, int iId);

  virtual ~TableAxis() {}

  /* Returns the label for a given axis position */
  virtual double label(int idx) const = 0;

  std::string label() const { return name_; }

  std::string id() const { return id_; }

  int iId() const { return iId_; }

protected:
  TableAxis(const std::string &name, const std::string &id, int iId)
      : name_(name), id_(id), iId_(iId) {}

  std::string name_;
  std::string id_;
  int iId_;
};

class TableDefinition {
public:
  // TableDefinition(int id, const std::string &name, const std::string
  // &description, TableType type, TableCategory category, DataType dataType,
  // uint32_t sizeX = 1, const TableAxis *axisX = nullptr, uint32_t sizeY = 1,
  // const TableAxis* axisY = nullptr, int min =
  // std::numeric_limits<int>::min(), int max =
  // std::numeric_limits<int>::max());

  TableDefinition(Definition *definition);

  TableDefinition();

  /* Attempts to load a table definition. Returns false and raises an
   * xml error on failure. */
  bool load(QXmlStreamReader &xml, Definition *def);

  // TODO: implement rvalue constructor

  std::string name() const { return name_; }

  std::string description() const { return description_; }

  TableType type() const { return type_; }

  TableCategory category() const { return category_; }

  uint32_t sizeX() const { return sizeX_; }

  uint32_t sizeY() const { return sizeY_; }

  int id() const { return id_; }

  DataType dataType() const { return dataType_; }

  int max() const { return max_; }

  int min() const { return min_; }

  const TableAxis *axisX() const;

  const TableAxis *axisY() const;

  bool valid() const { return definition_ != nullptr; }

private:
  int id_;
  std::string name_;
  std::string description_;
  TableType type_;
  TableCategory category_;
  DataType dataType_;
  uint32_t sizeX_;
  uint32_t sizeY_;
  int max_;
  int min_;
  std::string axisX_;
  std::string axisY_;

  Definition *definition_;
};

/**
 * Handles table definitions for each vehicle.
 */
class TableDefinitions {
public:
  /* Returns the amount of definitions */
  size_t count() const { return tables_.size(); }

  /* Returns an array of all table definitions */
  const TableDefinition *definitions() const { return tables_.data(); }

  /* Looks up a definition by id */
  const TableDefinition *at(int id) const;

  /* Add a table definition */
  void addTable(TableDefinition &&table);

  virtual ~TableDefinitions(){};

private:
  std::vector<TableDefinition> tables_;
};

#endif // TABLEDEFINITIONS_H
