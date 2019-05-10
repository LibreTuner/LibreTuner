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

#include "uds.h"

#include <cassert>

namespace lt {

UdsDtcScanner::UdsDtcScanner(network::UdsPtr &&uds) : uds_(std::move(uds)) {
    assert(uds_);
}

DiagnosticCodes UdsDtcScanner::scan() { return scanPid(0x3); }

DiagnosticCodes UdsDtcScanner::scanPending() { return scanPid(0x7); }

DiagnosticCodes UdsDtcScanner::scanPid(uint8_t pid) {
    // Scan with OBD-II Service 03
    // https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_03
    network::UdsPacket response = uds_->request(pid, nullptr, 0);

    DiagnosticCodes result;
    // Decode results as per
    // https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_03_(no_PID_required)
    for (size_t i = 1; i + 1 < response.data.size(); i += 2) {
        DiagnosticCode code;
        code.code = (response.data[i] << 8) | response.data[i + 1];

        result.emplace_back(std::move(code));
    }
    return result;
}

} // namespace lt
