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
#include <QXmlStreamReader>

#include "tablegroup.h"

namespace definition {
struct Model;
struct Main;
using ModelPtr = std::shared_ptr<Model>;
using PlatformPtr = std::shared_ptr<Main>;

struct Table;
}

class Table;
class Rom;

/**
 * TODO: write docs
 */
class Tune {
public:
    const std::string &name() const { return name_; }
    const std::string &path() const { return path_; }
    std::size_t id() const { return id_; }
    const std::shared_ptr<Rom> &base() const { return base_; }

    void setId(std::size_t id) { id_ = id; }
    void setName(const std::string &name) { name_ = name; }
    void setPath(const std::string &path) { path_ = path; }
    void setBase(const std::shared_ptr<Rom> &rom) { base_ = rom; }

    /* Applies table modifications to data and computes checksums.
     * Returns false on error and sets lastError. */
    void apply(uint8_t *data, size_t size);

    TableGroup &tables() { return tables_; }

    void save();

private:
    std::string name_;
    std::string path_;
    std::shared_ptr<Rom> base_;
    std::size_t id_;

    TableGroup tables_;
    void readTables(QXmlStreamReader &xml);
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
    const std::vector<std::shared_ptr<Tune>> &tunes() { return tunes_; }

    void setId(std::size_t id) { id_ = id; }
    void setName(const std::string &name) { name_ = name; }
    void setPath(const std::string &path) { path_ = path; }
    void setPlatform(const definition::PlatformPtr &platform) { platform_ = platform; }
    void setModel(const definition::ModelPtr &model) { model_ = model; }
    void addTune(const std::shared_ptr<Tune> &tune) { tunes_.push_back(std::move(tune)); }


private:
    std::string name_;
    std::string path_;
    definition::PlatformPtr platform_;
    definition::ModelPtr model_;
    std::size_t id_;

    std::vector<std::shared_ptr<Tune>> tunes_;
};



/* Loads a table from the rom. Returns nullptr if the
   table does not exist. */
std::unique_ptr<Table> loadTable(Rom &rom, std::size_t tableId);


#endif // ROM_H
