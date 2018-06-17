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

class Rom;
typedef std::shared_ptr<Rom> RomPtr;

class RomData;
typedef std::shared_ptr<RomData> RomDataPtr;

class TableGroup;
typedef std::shared_ptr<TableGroup> TableGroupPtr;

/**
 * TODO: write docs
 */
class Tune {
public:
  void setName(const std::string &name) { name_ = name; }

  const std::string &name() const { return name_; }

  void setPath(const std::string &path) { path_ = path; }

  const std::string &path() const { return path_; }

  RomPtr base() { return base_; }

  void setBase(RomPtr base) { base_ = base; }

private:
  std::string name_;
  std::string path_;

  RomPtr base_;
};
typedef std::shared_ptr<Tune> TunePtr;

class TuneData {
public:
  explicit TuneData(const TunePtr &tune);

  /* Applies table modifications to data and computes checksums.
   * Returns false on error and sets lastError. */
  bool apply(gsl::span<uint8_t> data);

  bool valid() const { return valid_; }

  std::string lastError() const { return lastError_; }

  TableGroupPtr tables() { return tables_; }

  TunePtr tune() { return tune_; }

  RomDataPtr romData() { return rom_; }

  bool save();

private:
  TunePtr tune_;
  RomDataPtr rom_;

  std::string lastError_;

  bool valid_;

  TableGroupPtr tables_;

  void readTables(QXmlStreamReader &xml);
};
typedef std::shared_ptr<TuneData> TuneDataPtr;

#endif // TUNE_H
