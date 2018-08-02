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

#ifndef LIBRETUNER_VEHICLE_H
#define LIBRETUNER_VEHICLE_H

#include <memory>
#include <string>

class Definition;
using DefinitionPtr = std::shared_ptr<Definition>;

class DataLogger;
using DataLoggerPtr = std::shared_ptr<DataLogger>;

class DataLink;
using DataLinkPtr = std::shared_ptr<DataLink>;

struct Vehicle {
  std::string name;
  std::string vin;
  DefinitionPtr definition;

  bool valid() const { return !vin.empty(); }

  static Vehicle fromVin(const std::string &vin);
};

// VehicleLink is DataLink + vehicle-specific optons (like the CAN bus baudrate)
class VehicleLink {
public:
    VehicleLink(const Vehicle &vehicle, const DataLinkPtr &link) : vehicle_(vehicle), datalink_(link) {}
    VehicleLink(Vehicle &&vehicle, const DataLinkPtr &link) : vehicle_(std::move(vehicle)), datalink_(link) {}

    /* Returns a logger suitable for logging from the vehicle using the datalink. Returns
       nullptr if a logger could not be created. */
    DataLoggerPtr logger();

private:
    DataLinkPtr datalink_;
    Vehicle vehicle_;
};

#endif // LIBRETUNER_VEHICLE_H
