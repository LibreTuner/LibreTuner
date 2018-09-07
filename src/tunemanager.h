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

#ifndef TUNEMANAGER_H
#define TUNEMANAGER_H

#include <memory>
#include <vector>

#include <QObject>
#include <QXmlStreamReader>

#include "tune.h"

class RomMeta;

/**
 * Manages tunes and tune metadata
 */
class TuneManager : public QObject {
    Q_OBJECT
public:
    /* Returns the global TuneManager object */
    static TuneManager *get();

    /* Loads tune metadata from storage. */
    void load();

    /* Saves tune data. */
    void save();

    const std::vector<TuneMeta> &tunes() const { return tunes_; }

    /* Creates a new tune with base 'base'. Returns the new tune.
     * If the tune could not be created, throw exception. */
    TuneMeta *createTune(const RomMeta &base, const std::string &name);

private:
    TuneManager();

    void readTunes(QXmlStreamReader &xml);

    std::vector<TuneMeta> tunes_;

signals:
    void updateTunes();
};

#endif // TUNEMANAGER_H
