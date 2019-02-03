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
#include "datalog/datalogger.h"
#include "definitions/definition.h"
#include "definitions/definitionmanager.h"
#include "diagnosticsinterface.h"
#include "flash/flasher.h"
#include "logger.h"
#include "protocols/isotpprotocol.h"
#include "protocols/udsprotocol.h"
#include "download/downloader.h"
#include <utility>


Vehicle Vehicle::fromVin(const std::string &vin) {
    std::shared_ptr<definition::Main> def = DefinitionManager::get()->fromVin(vin);
    if (def) {
        return Vehicle{def->name, vin, std::move(def)};
    }
    return Vehicle{vin, vin, nullptr};
}



std::unique_ptr<DataLogger> PlatformLink::logger(DataLog &log) const {
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



std::unique_ptr<download::Downloader> PlatformLink::downloader() const {
    if (!definition_) {
        return nullptr;
    }

    switch (definition_->downloadMode) {
    case DownloadMode::Mazda23:
        Logger::debug("Creating RMA downloader with key " + definition_->key + " and size " + std::to_string(definition_->romsize));
        return download::get_downloader("rma", *this, download::Options{definition_->key, definition_->romsize});
    default:
        return nullptr;
    }
}



std::unique_ptr<DiagnosticsInterface> PlatformLink::diagnostics() const {
    if (!definition_) {
        return nullptr;
    }

    if (auto iface = uds()) {
        return std::make_unique<UdsDiagnosticInterface>(std::move(iface));
    }
    return nullptr;
}



std::unique_ptr<flash::Flasher> PlatformLink::flasher() const {
    if (!definition_) {
        return nullptr;
    }
    switch (definition_->flashMode) {
    case FlashMode::T1:
        return flash::get_flasher("mazdat1", *this, flash::Options{definition_->key});
    default:
        return nullptr;
    }
}



std::unique_ptr<uds::Protocol> PlatformLink::uds() const {
    if (!definition_) {
        return nullptr;
    }

    if (auto interface = isotp()) {
        return std::make_unique<uds::IsoTpInterface>(std::move(interface));
    }
    return nullptr;
}



std::unique_ptr<isotp::Protocol> PlatformLink::isotp() const {
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



std::unique_ptr<CanInterface> PlatformLink::can() const {
    if (!definition_) {
        return nullptr;
    }
    return datalink_.can(definition_->baudrate);
}

PlatformLink::~PlatformLink() { Logger::debug("Closing vehicle link"); }
