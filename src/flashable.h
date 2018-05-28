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

#ifndef FLASHABLE_H
#define FLASHABLE_H

#include <memory>
#include <vector>

enum FlashMode {
  FLASH_NONE = 0,
  FLASH_T1, // Uses a CAN interface. Supported: Mazdaspeed 6
};

class TuneData;
typedef std::shared_ptr<TuneData> TuneDataPtr;

class SubDefinition;
typedef std::shared_ptr<SubDefinition> SubDefinitionPtr;

/**
 * A representation of data able to be flashed.
 */
class Flashable {
public:
  Flashable(TuneDataPtr tune);

  bool valid() const { return valid_; }

  // The address offset the data should be flashed to
  size_t offset() const { return offset_; }

  size_t size() const { return data_.size(); }

  const uint8_t *data() const { return data_.data(); }

  std::string lastError() const { return lastError_; }

  SubDefinitionPtr definition() const { return definition_; }

private:
  std::vector<uint8_t> data_;
  size_t offset_;
  bool valid_;
  SubDefinitionPtr definition_;

  std::string lastError_;
};

#endif // FLASHABLE_H
