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

#include "udsprotocol.h"
#include "caninterface.h"
#include "logger.h"

#include <array>
#include <cassert>
#include <utility>



namespace uds {

void IsoTpInterface::request(gsl::span<uint8_t> data, uint8_t expectedId, Packet &response) {
    isotp_->send(isotp::Packet(data));

    // Receive until we get a non-response-pending packet
    while (true) {
        // Receive response
        isotp::Packet res;
        isotp_->recv(res);

        if (res.eof()) {
            throw std::runtime_error("empty UDS response");
        }
        response.id = res.next();
        response.data = res.next(res.remaining());

        if (response.id == UDS_RES_NEGATIVE) {
            if (response.data.size() >= 2) {
                uint8_t code = response.data[1];
                if (code == UDS_NRES_RCRRP) {
                    // Response pending
                    continue;
                }
            }
            throw std::runtime_error("negative UDS response");
        }

        if (response.id != expectedId) {
            throw std::runtime_error("uds response id does not match expected id");
        }
        return;
    }
}



IsoTpInterface::IsoTpInterface(std::unique_ptr<isotp::Protocol> &&isotp)
    : isotp_(std::move(isotp)) {}



std::vector<uint8_t> Protocol::requestSession(uint8_t type) {
    std::array<uint8_t, 2> req = {UDS_REQ_SESSION, type};
    Packet packet;
    request(req, UDS_RES_SESSION, packet);


    if (packet.data.empty()) {
        throw std::runtime_error("empty DiagnosticSessionControl response");
    }

    if (packet.data[0] != type) {
        throw std::runtime_error("unequal diagnosticSessionType");
    }

    packet.data.erase(packet.data.begin());
    return packet.data;
}



std::vector<uint8_t> Protocol::requestSecuritySeed() {
  std::array<uint8_t, 2> req = {UDS_REQ_SECURITY, 1};
  Packet packet;
  request(req, UDS_RES_SECURITY, packet);

  if (packet.data.empty()) {
      throw std::runtime_error("empty SecurityAccess response");
  }

  if (packet.data[0] != 1) {
      throw std::runtime_error("securityAccessType does not match requestSeed");
  }

  packet.data.erase(packet.data.begin());
  return packet.data;
}



void Protocol::requestSecurityKey(gsl::span<uint8_t> key) {
    // Prepare request
    std::vector<uint8_t> req(key.size() + 2);
    req[0] = UDS_REQ_SECURITY;
    req[1] = 2;
    std::copy(key.begin(), key.end(), req.begin() + 2);

    Packet packet;
    // Send
    request(req, UDS_RES_SECURITY, packet);

    if (packet.data.empty()) {
        throw std::runtime_error("empty SecurityAccess response");
    }
}



std::vector<uint8_t> Protocol::requestReadMemoryAddress(uint32_t address, uint16_t length) {
  std::array<uint8_t, 7> req{};
  req[0] = UDS_REQ_READMEM;

  req[1] = (address & 0xFF000000) >> 24;
  req[2] = (address & 0xFF0000) >> 16;
  req[3] = (address & 0xFF00) >> 8;
  req[4] = address & 0xFF;

  req[5] = length >> 8;
  req[6] = length & 0xFF;

  Packet packet;
  request(req, UDS_RES_READMEM, packet);

  return packet.data;
}

} // namespace uds
