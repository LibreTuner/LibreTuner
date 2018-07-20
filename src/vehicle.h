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

class Vehicle;
using VehiclePtr = std::shared_ptr<Vehicle>;

class DataLogger;
using DataLoggerPtr = std::shared_ptr<DataLogger>;

class DataLink;
using DataLinkPtr = std::shared_ptr<DataLink>;

class VinInfo {
public:
private:
};

class Vehicle {
public:
  Vehicle(const std::string &name, const std::string &vin, DefinitionPtr ptr);
  Vehicle() = default;

  virtual ~Vehicle();

  std::string name() const { return name_; }

  std::string vin() const { return vin_; }

  // This function may return nullptr if the definition is undetermined
  DefinitionPtr definition() const { return definition_; }

  static VehiclePtr fromVin(const std::string &vin);

  /* Returns a logger suitable for logging from the vehicle using the datalink. Returns
     nullptr if a logger could not be created. */
  DataLoggerPtr logger(const DataLinkPtr &datalink);

private:
  std::string name_;
  std::string vin_;
  DefinitionPtr definition_;
};

#endif // LIBRETUNER_VEHICLE_H
