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

#include <cereal/cereal.hpp>

#include "../definition/model.h"
#include "../definition/platform.h"
#include "table.h"

namespace lt
{

class Rom
{
public:
    static constexpr auto extension = ".ltr";

    explicit Rom(ModelPtr model = ModelPtr()) : model_(std::move(model)) {}

    inline const std::string & name() const noexcept { return name_; }
    inline const ModelPtr & model() const noexcept { return model_; }
    inline Endianness endianness() const noexcept
    {
        return model_->platform.endianness;
    }
    inline const std::filesystem::path & path() const noexcept { return path_; }

    void setName(const std::string & name) { name_ = name; }
    void setModel(const ModelPtr & model) { model_ = model; }
    void setPath(std::filesystem::path path) { path_ = std::move(path); }

    inline const uint8_t * data() const noexcept { return data_.data(); }
    inline int size() const noexcept { return static_cast<int>(data_.size()); }

    std::vector<uint8_t>::iterator begin() noexcept { return data_.begin(); }
    std::vector<uint8_t>::iterator end() noexcept { return data_.end(); }
    std::vector<uint8_t>::const_iterator cbegin() const noexcept
    {
        return data_.cbegin();
    }
    std::vector<uint8_t>::const_iterator cend() const noexcept
    {
        return data_.cend();
    }

    // Sets the ROM data
    void setData(std::vector<uint8_t> && data) { data_ = std::move(data); }

    struct MetaData
    {
        std::string name;
        std::string platform;
        std::string model;

        template <class Archive>
        void serialize(Archive & archive, std::uint32_t const version)
        {
            archive(name, platform, model);
        }
    };

    // Constructs ROM metadata
    MetaData metadata() const noexcept;

    // Saves rom to `path_`
    void save() const;

private:
    std::string name_;
    ModelPtr model_;

    std::filesystem::path path_;

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
    static constexpr auto extension = ".ltr";

    explicit Tune(RomPtr rom) : base_(std::move(rom)) { assert(base_); }

    inline const std::string & name() const noexcept { return name_; }
    inline const RomPtr & base() const noexcept { return base_; }
    inline const std::filesystem::path & path() const noexcept { return path_; }

    // Returns true if any table is dirty
    bool dirty() const noexcept;

    // Clears dirty bit of all tables
    void clearDirty() noexcept;

    void setName(const std::string & name) { name_ = name; }
    void setBase(const RomPtr & rom) { base_ = rom; }
    void setPath(std::filesystem::path path) { path_ = std::move(path); }

    // Gets table by id. Returns nullptr if the table does not exist
    // If `create` is true and the table has not been initialized, creates
    // the table from the ROM data and definitions.
    Table * getTable(const std::string & id, bool create = true);

    Table * setTable(const std::string & id, const uint8_t * data,
                     std::size_t length);

    AxisPtr getAxis(const std::string & id, bool create = true);

    inline TableMap & tables() noexcept { return tables_; }
    inline const TableMap & tables() const noexcept { return tables_; }

    inline Endianness endianness() const noexcept
    {
        return base_->endianness();
    }

    struct MetaData
    {
        std::string name;
        // Base ROM id
        std::string base;

        template <class Archive>
        void serialize(Archive & archive, std::uint32_t const version)
        {
            archive(name, base);
        }
    };

    /* Constructs tune metadata */
    MetaData metadata() const noexcept;

    // Saves tune to `path_`
    void save() const;

private:
    std::string name_;

    RomPtr base_;
    TableMap tables_;

    std::unordered_map<std::string, AxisPtr> axes_;

    std::filesystem::path path_;
};
using TunePtr = std::shared_ptr<Tune>;

// Serialization
struct RomConstruct
{
    Rom::MetaData meta;
    std::vector<uint8_t> data;

    template <class Archive> void serialize(Archive & archive)
    {
        archive(meta, data);
    }
};

struct TableConstruct
{
    std::string id;
    std::vector<uint8_t> data;

    template <class Archive> void serialize(Archive & archive)
    {
        archive(id, data);
    }
};

struct TuneConstruct
{
    Tune::MetaData meta;
    std::vector<TableConstruct> tables;

    template <class Archive> void serialize(Archive & archive)
    {
        archive(meta, tables);
    }
};
} // namespace lt

// Declare cereal versions out of scope
CEREAL_CLASS_VERSION(lt::Rom::MetaData, 1)
CEREAL_CLASS_VERSION(lt::Tune::MetaData, 1)

#endif // ROM_H
