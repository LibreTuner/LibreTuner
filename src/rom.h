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
#include <unordered_map>

#include "tablegroup.h"
#include "flash/flashable.h"

class QXmlStreamReader;

namespace definition {
struct Model;
struct Main;
using ModelPtr = std::shared_ptr<Model>;
using PlatformPtr = std::shared_ptr<Main>;

struct Table;
}

class Table;
class Rom;
class RomData;

class TuneData {
public:
    TuneData(std::string path, Rom &base, bool open = true);
    
    /* Applies table modifications to data and computes checksums.
     * Returns false on error and sets lastError. */
    void apply(uint8_t *data, size_t size);

    TableGroup &tables() { return tables_; }

    void save();
    
    flash::Flashable flashable();
    
    bool dirty() const { return tables_.dirty(); }
    
private:
    void readTables(QXmlStreamReader &xml);
    
    std::string path_;
    Rom &base_;
    std::shared_ptr<RomData> baseData_;
    
    TableGroup tables_;
};

/**
 * TODO: write docs
 */
class Tune {
public:
    Tune();
    
    const std::string &name() const { return name_; }
    const std::string &path() const { return path_; }
    std::size_t id() const { return id_; }
    const Rom *base() const { return base_; }

    void setId(std::size_t id) { id_ = id; }
    void setName(const std::string &name) { name_ = name; }
    void setPath(const std::string &path) { path_ = path; }
    void setBase(Rom *rom) { base_ = rom; }
    
    // Returns the tune data, loading from file if needed. May throw an error.
    std::shared_ptr<TuneData> data();

private:
    std::string name_;
    std::string path_;
    Rom *base_ = nullptr;
    std::size_t id_;
    
    std::weak_ptr<TuneData> data_;
};

class RomData {
public:
    // Tries to open the file and read the rom data. May throw an exception.
    RomData(Rom &rom);
    
    const std::vector<uint8_t> &data() const { return data_; }
    
    // Sets the ROM data and saves to file
    void setData(std::vector<uint8_t> &&data);
    
    /* Loads a table from the rom. Returns nullptr if the
    table does not exist. */
    std::unique_ptr<Table> loadTable(std::size_t tableId);
    
    const Rom &rom() { return rom_; }
    
    // Returns nullptr if an axis with name `name` does not exist
    TableAxis *getAxis(const std::string &name);
    
private:
    Rom &rom_;
    std::vector<uint8_t> data_;
    std::unordered_map<std::string, std::unique_ptr<TableAxis>> axes_;
};



/* ROM Metadata */
class Rom {
public:
    Rom();

    const std::string &name() const { return name_; }
    const std::string &path() const { return path_; }
    const definition::PlatformPtr &platform() const { return platform_; }
    const definition::ModelPtr &model() const { return model_; }
    std::size_t id() const { return id_; }
    
    const std::vector<std::unique_ptr<Tune>> &tunes() { return tunes_; }
    
    std::size_t tunesCount() { return tunes_.size(); }
    
    // Returns tune at index `index` or nullptr if out of range
    Tune *getTune(std::size_t index);
    

    void setId(std::size_t id) { id_ = id; }
    void setName(const std::string &name) { name_ = name; }
    void setPath(const std::string &path) { path_ = path; }
    void setPlatform(const definition::PlatformPtr &platform) { platform_ = platform; }
    void setModel(const definition::ModelPtr &model) { model_ = model; }
    void addTune(std::unique_ptr<Tune> &&tune) { tunes_.push_back(std::move(tune)); }
    
    
    // Returns the ROM data, loading from file if needed. May throw an error.
    std::shared_ptr<RomData> data();


private:
    std::string name_;
    std::string path_;
    definition::PlatformPtr platform_;
    definition::ModelPtr model_;
    std::size_t id_;

    std::vector<std::unique_ptr<Tune>> tunes_;
    
    std::weak_ptr<RomData> data_;
};


#endif // ROM_H
