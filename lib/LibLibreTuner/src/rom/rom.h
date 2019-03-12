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
#include <unordered_map>
#include <vector>

#include "table.h"
#include "definition/model.h"

namespace lt {

/* ROM Metadata */
class Rom {
public:
    Rom();

    const std::string &name() const { return name_; }
    const std::string &path() const { return path_; }
    const ModelPtr &model() const { return model_; }
    std::size_t id() const { return id_; }

    void setId(std::size_t id) { id_ = id; }
    void setName(const std::string &name) { name_ = name; }
    void setPath(const std::string &path) { path_ = path; }
    void setModel(const ModelPtr &model) { model_ = model; }

    const std::vector<uint8_t> &data() const { return data_; }

    // Sets the ROM data
    void setData(std::vector<uint8_t> &&data) { data_ = std::move(data); }

	TablePtr baseTable(std::size_t tableId) const;

private:
    std::string name_;
    std::string path_;
    ModelPtr model_;
    std::size_t id_;

    // Tunes SHOULD NOT be tied to ROMs but instead to platforms
    // std::vector<std::unique_ptr<Tune>> tunes_;

    std::vector<uint8_t> data_;
};

class Tune {
public:
	Tune(Rom& rom) : base_(rom) {}

    const std::string &name() const { return name_; }
    const std::string &path() const { return path_; }
    std::size_t id() const { return id_; }
    const Rom &base() const { return base_; }

    void setId(std::size_t id) { id_ = id; }
    void setName(const std::string &name) { name_ = name; }
    void setPath(const std::string &path) { path_ = path; }
    void setBase(Rom &rom) { base_ = rom; }

	// Gets table by id. Returns nullptr if the table does not exist
	// If `create` is true and the table has not been initialized, creates
	// the table from the ROM data and definitions.
	Table *getTable(std::size_t id, bool create = true) const;

private:
    std::string name_;
    std::string path_;

    Rom &base_;
    std::vector<Table> tables_;

    std::size_t id_;
};

} // namespace lt

#endif // ROM_H
