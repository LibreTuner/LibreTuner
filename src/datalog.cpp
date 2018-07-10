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

#include "datalog.h"

bool DataLog::add(uint32_t id, std::pair<DataLog::TimePoint, double> value) {
  auto it = data_.find(id);
  if (it == data_.end()) {
    return false;
  }

  it->second.values.emplace_back(value);
  return true;
}

bool DataLog::add(uint32_t id, double value) {
  auto it = data_.find(id);
  if (it == data_.end()) {
    return false;
  }

  it->second.values.emplace_back(std::chrono::steady_clock::now(), value);
  return true;
}

void DataLog::addData(const DataLog::DataHead &data) {
    Data d;
    d.head = data;
  data_.emplace(data.id, std::move(d));
}
