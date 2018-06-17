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

#include "vehicle.h"

class DataLog {
public:
  std::chrono::system_clock::time_point creationTime() const {
    return creationTime_;
  }

  VehiclePtr vehicle() const { return vehicle_; }

private:
  std::chrono::system_clock::time_point creationTime_;
  VehiclePtr vehicle_;
};

#endif // LIBRETUNER_DATALOG_H
