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

#ifndef TUNE_H
#define TUNE_H

#include <gsl/span>
#include <memory>
#include <string>

#include <QXmlStreamReader>

#include "tablegroup.h"

class Rom;

/**
 * TODO: write docs
 */
struct TuneMeta {
    std::string name;
    std::string path;
    int baseId;
};



class Tune {
public:
    explicit Tune(const TuneMeta &tune);

    /* Applies table modifications to data and computes checksums.
     * Returns false on error and sets lastError. */
    void apply(gsl::span<uint8_t> data);

    TableGroup tables() { return tables_; }

    const std::shared_ptr<Rom> &rom() { return rom_; }

    void save();

private:
    TuneMeta meta_;

    std::shared_ptr<Rom> rom_;

    TableGroup tables_;

    void readTables(QXmlStreamReader &xml);
};


#endif // TUNE_H
