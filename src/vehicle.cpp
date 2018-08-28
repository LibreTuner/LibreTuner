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
#include "diagnosticsinterface.h"
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



std::unique_ptr<DataLogger> VehicleLink::logger() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }

    switch (vehicle_.definition->logMode()) {
    case LogMode::Uds: {
        if (auto interface = uds()) {
            return std::make_unique<UdsDataLogger>(std::move(interface));
        }
        return nullptr;
    }
    default:
        return nullptr;
    }
}



std::unique_ptr<DownloadInterface> VehicleLink::downloader() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }

    switch (vehicle_.definition->downloadMode()) {
    case DM_MAZDA23:
        if (auto interface = uds()) {
            return std::make_unique<Uds23DownloadInterface>(std::move(interface), vehicle_.definition->key(), vehicle_.definition->size());
        }
    default:
        return nullptr;
    }
}

std::unique_ptr<DiagnosticsInterface> VehicleLink::diagnostics() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }

    if (auto iface = uds()) {
        return std::make_unique<UdsDiagnosticInterface>(std::move(iface));
    }
    return nullptr;
}



std::unique_ptr<Flasher> VehicleLink::flasher() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }
    switch (vehicle_.definition->flashMode()) {
    case FLASH_T1:
        if (auto interface = uds()) {
            return std::make_unique<MazdaT1Flasher>(vehicle_.definition->key(), std::move(interface));
        }
    default:
        return nullptr;
    }
    return nullptr;
}



std::unique_ptr<uds::Protocol> VehicleLink::uds() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }

    if (auto interface = isotp()) {
        return std::make_unique<uds::IsoTpInterface>(std::move(interface));
    }
    return nullptr;
}



std::unique_ptr<isotp::Protocol> VehicleLink::isotp() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }
    if (auto c = can()) {
        return std::make_unique<isotp::Protocol>(std::move(c), isotp::Options{vehicle_.definition->serverId(), vehicle_.definition->serverId() + 8, std::chrono::milliseconds{8000}});
    }
    return nullptr;
}



std::unique_ptr<CanInterface> VehicleLink::can() const
{
    assert(datalink_);
    if (!vehicle_.definition) {
        return nullptr;
    }
    return datalink_->can(vehicle_.definition->baudrate());
}
