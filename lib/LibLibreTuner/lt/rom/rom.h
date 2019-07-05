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

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../definition/model.h"
#include "../definition/platform.h"
#include "table.h"

namespace lt
{

class Rom
{
public:
    explicit Rom(ModelPtr model = ModelPtr()) : model_(std::move(model)) {}

    inline const std::string & name() const noexcept { return name_; }
    inline const ModelPtr & model() const noexcept { return model_; }
    inline const std::string & id() const noexcept { return id_; }
    inline Endianness  endianness() const noexcept { return model_->platform.endianness; }

    void setId(const std::string & id) { id_ = id; }
    void setName(const std::string & name) { name_ = name; }
    void setModel(const ModelPtr & model) { model_ = model; }

    inline const uint8_t * data() const noexcept { return data_.data(); }
    inline int size() const noexcept { return static_cast<int>(data_.size()); }

    std::vector<uint8_t>::iterator begin() noexcept { return data_.begin(); }
    std::vector<uint8_t>::iterator end() noexcept { return data_.end(); }
    std::vector<uint8_t>::const_iterator cbegin() const noexcept { return data_.cbegin(); }
    std::vector<uint8_t>::const_iterator cend() const noexcept { return data_.cend(); }

    // Sets the ROM data
    void setData(std::vector<uint8_t> && data) { data_ = std::move(data); }

private:
    std::string name_;
    ModelPtr model_;
    std::string id_;

    // Tunes SHOULD NOT be tied to ROMs but instead to platforms
    // std::vector<std::unique_ptr<Tune>> tunes_;

    std::vector<uint8_t> data_;
};
using RomPtr = std::shared_ptr<Rom>;
using WeakRomPtr = std::weak_ptr<Rom>;

using TableMap = std::unordered_map<std::string, std::unique_ptr<Table>>;

class Tune
{
public:
    explicit Tune(RomPtr rom) : base_(std::move(rom)) { assert(base_); }

    inline const std::string & name() const noexcept { return name_; }
    inline const std::string & id() const noexcept { return id_; }
    inline const RomPtr & base() const noexcept { return base_; }

    // Returns true if any table is dirty
    bool dirty() const noexcept;

    // Clears dirty bit of all tables
    void clearDirty() noexcept;

    void setId(const std::string & id) { id_ = id; }
    void setName(const std::string & name) { name_ = name; }
    void setBase(const RomPtr & rom) { base_ = rom; }

    // Gets table by id. Returns nullptr if the table does not exist
    // If `create` is true and the table has not been initialized, creates
    // the table from the ROM data and definitions.
    Table * getTable(const std::string & id, bool create = true);

    Table * setTable(const std::string & id, const uint8_t * data,
                     std::size_t length);

    AxisPtr getAxis(const std::string & id, bool create = true);

    inline TableMap & tables() noexcept { return tables_; }
    inline const TableMap & tables() const noexcept { return tables_; }

    inline Endianness endianness() const noexcept { return base_->endianness(); }

    TablePtr deserializeTable(const TableDefinition & def, const uint8_t * data,
                              std::size_t length);

private:
    std::string name_;

    RomPtr base_;
    TableMap tables_;

    std::unordered_map<std::string, AxisPtr> axes_;

    std::string id_;
};
using TunePtr = std::shared_ptr<Tune>;

class Platforms;

/* Manages saving and loading of ROMs and tunes from files.
 * `FileRomDatabase` loads and stores ROMs in a specified
 * directory. */
class FileRomDatabase
{
public:
    // Initializes base path for ROM storage.
    FileRomDatabase(std::filesystem::path base,
                    const Platforms & platforms)
        : base_(base), platforms_(std::move(platforms))
    {
    }

    /* Loads a ROM by id. If the ROM is cached, it will be returned.
     * Otherwise, the directory is searched and if the ROM cannot
     * be found, RomPtr() is returned. If the ROM was found but deserialization
     * fails, throws an exception. */
    RomPtr getRom(const std::string & id);

    /* Saves ROM to file. Throws an exception if the ROM could not be saved
     * (i.e. invalid id or a ROM with the id already exists) */
    void saveRom(const Rom &rom);

    /* Loads tune from a path. Returns a null pointer if the path does not
     * exist. Throws an exception if it cannot be deserialized or the base
     * cannot be found. */
    TunePtr loadTune(const std::filesystem::path & path);

    /* Saves a tune to a file. Throws an exception if serialization failed or
     * the file couldn't be opened. */
    void saveTune(const Tune &tune, std::filesystem::path & path);

private:
    std::filesystem::path base_;
    std::unordered_map<std::string, WeakRomPtr> cache_;
    const Platforms & platforms_;
};

} // namespace lt

#endif // ROM_H
