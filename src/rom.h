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

class Definition;
typedef std::shared_ptr<Definition> DefinitionPtr;

class SubDefinition;
typedef std::shared_ptr<SubDefinition> SubDefinitionPtr;

/* ROM Metadata */
struct RomMeta {
    std::string name;
    std::string path;
    std::string definitionId;
    std::string subDefinitionId;
    int id;
};

class Table;
typedef std::shared_ptr<Table> TablePtr;

/* The object that actually stores firmware data. */
class Rom {
public:
    explicit Rom(const RomMeta &rom);

    /* Returns the base table from the table index */
    TablePtr getTable(int idx);

    const std::vector<uint8_t> data() { return data_; }

    std::string name() { return name_; }

    DefinitionPtr definition() const { return definition_; }

    SubDefinitionPtr subDefinition() const { return subDefinition_; }

private:
    std::string name_;

    DefinitionPtr definition_;
    SubDefinitionPtr subDefinition_;

    /* Raw firmware data */
    std::vector<uint8_t> data_;
};

#endif // ROM_H
