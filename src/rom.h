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

namespace definition {
struct Model;
using ModelPtr = std::shared_ptr<Model>;

struct Table;
}

class Table;

/* ROM Metadata */
struct RomMeta {
    std::string name;
    std::string path;
    std::string definitionId;
    std::string modelId;
    int id;

    std::vector<int> tunes;
};

/* The object that actually stores firmware data. */
class Rom {
public:
    explicit Rom(const RomMeta &rom);

    /* Returns the raw ROM */
    const std::vector<uint8_t> &data() { return data_; }

    std::string name() { return name_; }

    definition::ModelPtr definition() const { return definition_; }

private:
    std::string name_;

    definition::ModelPtr definition_;

    /* Raw firmware data */
    std::vector<uint8_t> data_;
};

/* Loads a table from the definition. Returns nullptr if the
   table does not exist. */
std::unique_ptr<Table> loadTable(Rom &rom, std::size_t tableId);

#endif // ROM_H
