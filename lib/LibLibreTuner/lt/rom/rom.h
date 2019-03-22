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
#include "../definition/model.h"

namespace lt {

/* ROM Metadata */
class Rom {
public:
	Rom(ModelPtr model = ModelPtr()) : model_(std::move(model)) {}

    const std::string &name() const { return name_; }
    const ModelPtr &model() const { return model_; }
    const std::string &id() const { return id_; }

    void setId(const std::string &id) { id_ = id; }
    void setName(const std::string &name) { name_ = name; }
    void setModel(const ModelPtr &model) { model_ = model; }

    inline const uint8_t *data() const noexcept { return data_.data(); }
    inline std::size_t size() const noexcept { return data_.size(); }

    // Sets the ROM data
    void setData(std::vector<uint8_t> &&data) { data_ = std::move(data); }
    
    std::vector<uint8_t> getRawTableData(std::size_t id) const;
    std::vector<uint8_t> getRawTableData(const ModelTable *modTable) const;
  
    TablePtr baseTable(std::size_t tableId) const;

    TablePtr deserializeTable(std::size_t tableId, const uint8_t *data, std::size_t length) const;

private:
    std::string name_;
    ModelPtr model_;
    std::string id_;

    // Tunes SHOULD NOT be tied to ROMs but instead to platforms
    // std::vector<std::unique_ptr<Tune>> tunes_;

    std::vector<uint8_t> data_;
};
using RomPtr = std::shared_ptr<Rom>;

class Tune {
public:
	Tune(RomPtr rom) : base_(std::move(rom)) {
        assert(base_);
	}

    inline const std::string &name() const noexcept { return name_; }
    inline const std::string &id() const noexcept { return id_; }
    inline const RomPtr &base() const noexcept { return base_; }

	// Returns true if any table is dirty
	bool dirty() const noexcept;

	// Clears dirty bit of all tables
	void clearDirty() noexcept;

    void setId(const std::string &id) { id_ = id; }
    void setName(const std::string &name) { name_ = name; }
    void setBase(const RomPtr &rom) { base_ = rom; }

	// Gets table by id. Returns nullptr if the table does not exist
	// If `create` is true and the table has not been initialized, creates
	// the table from the ROM data and definitions.
    Table *getTable(std::size_t id, bool create = true);

    Table *setTable(std::size_t id, const uint8_t *data, std::size_t length);

	TableAxisPtr getAxis(const std::string &id, bool create = true);

	inline std::vector<std::unique_ptr<Table>> &tables() noexcept { return tables_; }
	inline const std::vector<std::unique_ptr<Table>>& tables() const noexcept { return tables_; }

private:
    std::string name_;

    RomPtr base_;
    std::vector<std::unique_ptr<Table>> tables_;

	std::unordered_map<std::string, TableAxisPtr> axes_;

    std::string id_;
};
using TunePtr = std::shared_ptr<Tune>;

} // namespace lt

#endif // ROM_H
