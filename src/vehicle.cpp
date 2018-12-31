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

#include "datalink/datalink.h"
#include "datalogger.h"
#include "definitions/definition.h"
#include "definitions/definitionmanager.h"
#include "diagnosticsinterface.h"
#include "flasher.h"
#include "logger.h"
#include "protocols/isotpprotocol.h"
#include "protocols/udsprotocol.h"
#include "downloadinterface.h"
#include <utility>


Vehicle Vehicle::fromVin(const std::string &vin) {
    std::shared_ptr<definition::Main> def = DefinitionManager::get()->fromVin(vin);
    if (def) {
        return Vehicle{def->name, vin, std::move(def)};
    }
    return Vehicle{vin, vin, nullptr};
}



std::unique_ptr<DataLogger> VehicleLink::logger(DataLog &log) const {
    if (!definition_) {
        return nullptr;
    }

    switch (definition_->logMode) {
    case LogMode::Uds: {
        if (auto interface = uds()) {
            return std::make_unique<UdsDataLogger>(log, std::move(interface));
        }
        return nullptr;
    }
    default:
        return nullptr;
    }
}



std::unique_ptr<DownloadInterface> VehicleLink::downloader() const {
    if (!definition_) {
        return nullptr;
    }

    switch (definition_->downloadMode) {
    case DownloadMode::Mazda23:
        if (auto interface = uds()) {
            return std::make_unique<Uds23DownloadInterface>(
                std::move(interface), definition_->key,
                definition_->romsize);
        }
    default:
        return nullptr;
    }
}



std::unique_ptr<DiagnosticsInterface> VehicleLink::diagnostics() const {
    if (!definition_) {
        return nullptr;
    }

    if (auto iface = uds()) {
        return std::make_unique<UdsDiagnosticInterface>(std::move(iface));
    }
    return nullptr;
}



std::unique_ptr<Flasher> VehicleLink::flasher() const {
    if (!definition_) {
        return nullptr;
    }
    switch (definition_->flashMode) {
    case FlashMode::T1:
        if (auto interface = uds()) {
            return std::make_unique<MazdaT1Flasher>(definition_->key,
                                                    std::move(interface));
        }
    default:
        return nullptr;
    }
}



std::unique_ptr<uds::Protocol> VehicleLink::uds() const {
    if (!definition_) {
        return nullptr;
    }

    if (auto interface = isotp()) {
        return std::make_unique<uds::IsoTpInterface>(std::move(interface));
    }
    return nullptr;
}



std::unique_ptr<isotp::Protocol> VehicleLink::isotp() const {
    if (!definition_) {
        return nullptr;
    }
    if (auto c = can()) {
        return std::make_unique<isotp::Protocol>(
            std::move(c), isotp::Options{definition_->serverId,
                                         definition_->serverId + 8,
                                         std::chrono::milliseconds{8000}});
    }
    return nullptr;
}



std::unique_ptr<CanInterface> VehicleLink::can() const {
    if (!definition_) {
        return nullptr;
    }
    return datalink_.can(definition_->baudrate);
}

VehicleLink::~VehicleLink() { Logger::debug("Closing vehicle link"); }
