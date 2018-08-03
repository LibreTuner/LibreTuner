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

#include <utility>
#include "definitions/definitionmanager.h"
#include "definitions/definition.h"
#include "protocols/isotpprotocol.h"
#include "protocols/udsprotocol.h"
#include "datalink.h"
#include "datalogger.h"
#include "flasher.h"


Vehicle Vehicle::fromVin(const std::string &vin) {
  DefinitionPtr def = DefinitionManager::get()->fromVin(vin);
  if (def) {
      return Vehicle{def->name(), vin, std::move(def)};
  }
  return Vehicle{vin, vin, nullptr};
}

DataLoggerPtr VehicleLink::logger() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }

    switch (vehicle_.definition->logMode()) {
    case LogMode::Uds: {
        if (const auto &interface = isotp()) {
            std::shared_ptr<uds::Protocol> uds = uds::Protocol::create(interface);
            return std::static_pointer_cast<DataLogger>(std::make_shared<UdsDataLogger>(std::move(uds)));
        }
        return nullptr;
    }
    default:
        return nullptr;
    }
}

DownloadInterfacePtr VehicleLink::downloader() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }

    switch (vehicle_.definition->downloadMode()) {
    case DM_MAZDA23:
        if (const auto &interface = isotp()) {
            return DownloadInterface::createM23(interface, vehicle_.definition->key(), vehicle_.definition->size());
        }
    default:
        return nullptr;
    }
}

FlasherPtr VehicleLink::flasher() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }
    switch (vehicle_.definition->flashMode()) {
    case FLASH_T1:
        if (const auto &interface = isotp()) {
            return Flasher::createT1(vehicle_.definition->key(), interface);
        }
    default:
        return nullptr;
    }
    return nullptr;
}

std::shared_ptr<isotp::Protocol> VehicleLink::isotp() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }
    if (const auto &c = can()) {
        return std::make_shared<isotp::Protocol>(c, isotp::Options{vehicle_.definition->serverId(), vehicle_.definition->serverId() + 1, std::chrono::milliseconds{100}});
    }
    return nullptr;
}

CanInterfacePtr VehicleLink::can() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }
    return datalink_->can(vehicle_.definition->baudrate());
}
