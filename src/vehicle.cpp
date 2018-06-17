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

#include "vehicle.h"
#include "definitions/definitionmanager.h"

Vehicle::Vehicle(const std::string &name, const std::string &vin,
                 DefinitionPtr ptr) {}

struct make_shared_enabler : public Vehicle {
  template <typename... Args>
  explicit make_shared_enabler(Args &&... args)
      : Vehicle(std::forward<Args>(args)...) {}
};

VehiclePtr Vehicle::fromVin(const std::string &vin) {
  DefinitionPtr def = DefinitionManager::get()->fromVin(vin);
  if (def) {
    return std::make_shared<make_shared_enabler>(def->name(), vin, def);
  }
  return std::make_shared<make_shared_enabler>(vin, vin, nullptr);
}
