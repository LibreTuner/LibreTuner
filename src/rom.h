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

#ifndef ROM_H
#define ROM_H

#include <memory>
#include <string>
#include <vector>

class RomData;
typedef std::shared_ptr<RomData> RomDataPtr;

class Definition;
typedef std::shared_ptr<Definition> DefinitionPtr;

class SubDefinition;
typedef std::shared_ptr<SubDefinition> SubDefinitionPtr;

/* ROM Metadata */
class Rom {
public:
  void setName(const std::string &name) { name_ = name; }

  std::string name() const { return name_; }

  void setPath(const std::string &path) { path_ = path; }

  std::string path() const { return path_; }

  int id() const { return id_; }

  void setId(int id) { id_ = id; }

  std::string definitionId() const { return definitionId_; }

  void setDefinition(const std::string &id) { definitionId_ = id; }

  std::string subDefinitionId() const { return subDefinitionId_; }

  void setSubDefinition(const std::string &id) { subDefinitionId_ = id; }

private:
  std::string name_;
  std::string path_;
  std::string definitionId_;
  std::string subDefinitionId_;
  int id_;
};
typedef std::shared_ptr<Rom> RomPtr;

class Table;
typedef std::shared_ptr<Table> TablePtr;

/* The object that actually stores firmware data. */
class RomData {
public:
  RomData(RomPtr rom);

  /* Returns the base table from the table index */
  TablePtr getTable(int idx);

  bool valid() const { return valid_; }

  std::string lastError() const { return lastError_; }

  const uint8_t *data() const { return data_.data(); }

  uint8_t *data() { return data_.data(); }

  size_t size() const { return data_.size(); }

  RomPtr rom() const { return rom_; }

  DefinitionPtr definition() const { return definition_; }

  SubDefinitionPtr subDefinition() const { return subDefinition_; }

private:
  RomPtr rom_;

  bool valid_;
  std::string lastError_;

  DefinitionPtr definition_;
  SubDefinitionPtr subDefinition_;

  /* Raw firmware data */
  std::vector<uint8_t> data_;
};

#endif // ROM_H
