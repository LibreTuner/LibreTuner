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

#ifndef ROMMANAGER_H
#define ROMMANAGER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "rom.h"

class TuneManager;

namespace definition {
struct Main;
using MainPtr = std::shared_ptr<Main>;
}

/**
 * Manages ROM files and metadata
 */
class RomStore : public QObject {
    Q_OBJECT
public:
    static RomStore *get();

    RomStore(const RomStore &) = delete;
    RomStore &operator=(const RomStore &) = delete;

    /* Loads rom list and metadata. */
    void load();

    /* Saves rom list and metadata */
    void save();

    const std::vector<std::shared_ptr<Rom>> &roms() { return roms_; }

    void addRom(const std::string &name, const definition::MainPtr &definition,
                const uint8_t *data, size_t size);

    /* Returns the ROM with id or nullptr if the ROM does
     * not exist. Be careful not to store this reference
     * as ROMs can be added or removed. */
    std::shared_ptr<Rom> fromId(std::size_t id);

    /* Creates a new tune with base 'base'. Returns the new tune.
     * If the tune could not be created, throw exception. */
    std::shared_ptr<Tune> createTune(const std::shared_ptr<Rom> &base, const std::string &name);

    void readTunes(QXmlStreamReader &xml);

    void saveTunes();

    void loadTunes();

private:
    RomStore() = default;
    std::vector<std::shared_ptr<Rom>> roms_;
    std::size_t nextId_{};

    void readRoms(QXmlStreamReader &xml);

signals:
    void updateRoms();
};

#endif // ROMMANAGER_H
