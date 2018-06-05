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
#include <gsl/span>

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "rom.h"


/**
 * Manages ROM files and metadata
 */
class RomManager : public QObject {
  Q_OBJECT
public:
  static RomManager *get();

  /* Loads rom list and metadata. Returns true if no errors
   * occurred */
  bool load();

  /* Saves rom list and metadata */
  bool save();

  QString lastError() const { return lastError_; }

  /* Returns the amount of roms */
  size_t count() const { return roms_.size(); }

  std::vector<RomPtr> &roms() { return roms_; }

  bool addRom(const std::string &name, const DefinitionPtr &definition,
              gsl::span<const uint8_t> data);

  /* Returns the ROM with id or nullptr if the ROM does
   * not exist */
  RomPtr fromId(int id);

private:
  RomManager() = default;
  QString lastError_;
  std::vector<RomPtr> roms_;
  int nextId_{};

  void readRoms(QXmlStreamReader &xml);

signals:
  void updateRoms();
};

#endif // ROMMANAGER_H
