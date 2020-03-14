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
#include "../buffer/memorybuffer.h"
#include "table.h"

namespace lt
{

class Calibration
{
public:
    static constexpr auto extension = ".ltr";

    explicit Calibration(const Model * model = nullptr) : model_(model) {}

    inline const std::string & name() const noexcept { return name_; }
    inline Model const * model() const noexcept { return model_; }

    // Returns the endianness of the platform.
    inline Endianness endianness() const noexcept { return model_->platform().endianness; }
    inline const std::filesystem::path & path() const noexcept { return path_; }

    void setName(const std::string & name) { name_ = name; }
    void setModel(const Model * model) { model_ = model; }
    void setPath(std::filesystem::path path) { path_ = std::move(path); }

    inline const uint8_t * data() const noexcept { return data_.data(); }
    inline int size() const noexcept { return static_cast<int>(data_.size()); }

    std::vector<uint8_t>::iterator begin() noexcept { return data_.begin(); }
    std::vector<uint8_t>::iterator end() noexcept { return data_.end(); }
    std::vector<uint8_t>::const_iterator cbegin() const noexcept { return data_.cbegin(); }
    std::vector<uint8_t>::const_iterator cend() const noexcept { return data_.cend(); }

    // Sets the ROM data
    void setData(MemoryBuffer && data) { data_ = std::move(data); }
    View view(int offset, int size) { return data_.view(offset, size); }
    View view() { return data_.view(); }

    // Gets table by id. Returns none if the table does not exist.
    // If `create` is true and the table has not been initialized, creates
    // the table from the ROM data and definitions.
    std::optional<Table> getTable(const std::string & id, bool create = true);

    std::unique_ptr<Axis> getAxis(const std::string & id, bool create = true);

    struct MetaData
    {
        std::string name;
        std::string platform;
        std::string model;

        // Path is set after loading
        std::filesystem::path path;

        template <class Archive>
        void serialize(Archive & archive, std::uint32_t const /*version*/)
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
    const Model * model_;

    std::filesystem::path path_;

    MemoryBuffer data_;
};
using RomPtr = std::shared_ptr<Calibration>;
using WeakRomPtr = std::weak_ptr<Calibration>;

/*
class Tune
{
public:
    using iterator = MemoryBuffer::iterator;
    using const_iterator = MemoryBuffer::const_iterator;

    static constexpr auto extension = ".ltt";

    explicit Tune(RomPtr rom);
    explicit Tune(RomPtr rom, MemoryBuffer && data);

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
        std::filesystem::path path;

        template <class Archive>
        void serialize(Archive & archive, std::uint32_t const _version)
        {
            archive(name, base);
        }
    };

    // Constructs tune metadata
    MetaData metadata() const noexcept;

    // Saves tune to `path_`
    void save() const;

    inline iterator begin() { return data_.begin(); }
    inline const_iterator cbegin() const { return data_.cbegin(); };
    inline iterator end() { return data_.end(); }
    inline const_iterator cend() { return data_.cend(); }
    inline std::vector<uint8_t>::size_type size() const { return data_.size(); }

private:
    std::string name_;

    RomPtr base_;
    TableMap tables_;

    MemoryBuffer data_;

    std::unordered_map<std::string, AxisPtr> axes_;

    std::filesystem::path path_;
};
using TunePtr = std::shared_ptr<Tune>;
using WeakTunePtr = std::weak_ptr<Tune>;
 */

} // namespace lt

#endif // ROM_H
