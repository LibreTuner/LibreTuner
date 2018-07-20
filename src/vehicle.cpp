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
#include "definitions/definition.h"
#include "protocols/isotpprotocol.h"
#include "protocols/udsprotocol.h"
#include "datalink.h"
#include "datalogger.h"

Vehicle::Vehicle(const std::string &name, const std::string &vin,
                 DefinitionPtr ptr) : name_(name), vin_(vin), definition_(std::move(ptr)) {}

Vehicle::~Vehicle()
{

}

struct make_shared_enabler : public Vehicle {
  template <typename... Args>
  explicit make_shared_enabler(Args &&... args)
      : Vehicle(std::forward<Args>(args)...) {}
};

VehiclePtr Vehicle::fromVin(const std::string &vin) {
  DefinitionPtr def = DefinitionManager::get()->fromVin(vin);
  if (def) {
    return std::make_shared<Vehicle>(def->name(), vin, def);
  }
  return std::make_shared<Vehicle>(vin, vin, nullptr);
}

DataLoggerPtr Vehicle::logger(const DataLinkPtr &datalink)
{
    assert(datalink);
    if (!definition_) {
        return nullptr;
    }

    switch (definition_->logMode()) {
    case LogMode::Uds: {
        // Obtain a CAN interface and check if it's valid
        CanInterfacePtr can = datalink->can();
        if (!can) {
            return nullptr;
        }

        // Create the ISO-TP interface using the options from the definition
        std::shared_ptr<isotp::Protocol> isotp = std::make_shared<isotp::Protocol>(can, isotp::Options{definition_->serverId(), definition_->serverId() + 8});
        std::shared_ptr<uds::Protocol> uds = uds::Protocol::create(isotp);

        return std::static_pointer_cast<DataLogger>(std::make_shared<UdsDataLogger>(std::move(uds)));
    }
    default:
        return nullptr;
    }
}
