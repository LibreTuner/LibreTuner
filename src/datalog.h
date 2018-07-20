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

#ifndef LIBRETUNER_DATALOG_H
#define LIBRETUNER_DATALOG_H

#include <chrono>
#include <vector>
#include <unordered_map>

#include "vehicle.h"

enum class DataUnit {
  None,
  Percentage,
  Degrees,
};

class DataLog {
public:
  using TimePoint = std::chrono::steady_clock::time_point;
  struct DataHead {
    std::string name;
    std::string description;
    std::string formula;
    uint32_t id;
    DataUnit unit;
  };

  struct Data {
    DataHead head;
    std::vector<std::pair<TimePoint, double>> values;
  };

  std::chrono::system_clock::time_point creationTime() const {
    return creationTime_;
  }

  VehiclePtr vehicle() const { return vehicle_; }

  /* adds a point to a dataset. Returns false if the dataset
   * with the specified id does not exist. */
  bool add(uint32_t id, std::pair<TimePoint, double> value);

  /* Adds a value at the current time */
  bool add(uint32_t id, double value);

  void addData(const DataHead &data);

private:
  std::chrono::system_clock::time_point creationTime_;
  VehiclePtr vehicle_;

  std::unordered_map<uint32_t, Data> data_;
};
using DataLogPtr = std::shared_ptr<DataLog>;

#endif // LIBRETUNER_DATALOG_H
