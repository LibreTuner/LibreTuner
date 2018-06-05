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

#include <cassert>
#include <utility>
#include <array>

namespace uds {
std::string strError(Error error) {
  switch (error) {
    case Error::Success:
      return "success";
    case Error::IsoTp:
      return "ISO-TP error";
    case Error::Timeout:
      return "timed out";
    case Error::BlankResponse:
      return "received a blank response";
    case Error::Consec:
      return "ISO-TP consecutive index invalid in frame";
    case Error::Negative:
      return "negative response code received";
    case Error::Malformed:
      return "malformed UDS response";
    case Error::UnexpectedResponse:
      return "unexpected response";
    default:
      return "unknown";
  }
  return "you should never see this";
}

class IsoTpInterface : public Protocol {
public:
  explicit IsoTpInterface(std::shared_ptr<isotp::Protocol> isotp);

  void request(gsl::span<uint8_t>, uint8_t expectedId, Callback &&cb) override;

private:
  std::shared_ptr<isotp::Protocol> isotp_;
};

void IsoTpInterface::request(gsl::span<uint8_t> data, uint8_t expectedId, Callback &&cb) {
  isotp_->request(isotp::Packet(data), [this, cb, expectedId](isotp::Error error, isotp::Packet &&packet) {
    if (error != isotp::Error::Success) {
      switch (error) {
        case isotp::Error::Timeout:
          cb(Error::Timeout, Packet{});
          break;
        case isotp::Error::Consec:
          cb(Error::Consec, Packet{});
          break;
        default:
          cb(Error::IsoTp, Packet{});
          break;
      }
      return;
    }

    Packet res;
    packet.moveAll(res.data);
    if (res.data.empty()) {
      cb(Error::BlankResponse, Packet{});
      return;
    }
    res.id = res.data[0];
    if (res.id == UDS_RES_NEGATIVE) {
      cb(Error::Negative, Packet{});
      return;
    }
    if (res.id != expectedId) {
      cb(Error::UnexpectedResponse, Packet{});
      return;
    }
    res.data.erase(std::begin(res.data));
    cb(Error::Success, res);
  });
}

IsoTpInterface::IsoTpInterface(std::shared_ptr<isotp::Protocol> isotp)
    : isotp_(std::move(isotp)) {

}

std::shared_ptr<Protocol>
Protocol::create(std::shared_ptr<isotp::Protocol> isotp) {
  return std::make_shared<IsoTpInterface>(std::move(isotp));
}

void Protocol::requestSession(uint8_t type, RequestSessionCallback &&cb) {
  std::array<uint8_t, 2> req = {UDS_REQ_SESSION, type};
  request(req, UDS_RES_SESSION, [cb{std::move(cb)}] (Error error, const Packet &packet) {
    if (error != Error::Success) {
      cb(error, 0, gsl::span<uint8_t>());
      return;
    }

    if (packet.data.empty()) {
      cb(Error::Malformed, 0, gsl::span<uint8_t>());
      return;
    }

    cb(Error::Success, packet.data[0], gsl::make_span(packet.data).subspan(1));
  });
}

void Protocol::requestSecuritySeed(RequestSecuritySeedCallback &&cb) {
  std::array<uint8_t, 2> req  = {UDS_REQ_SECURITY, 1};
  return request(req, UDS_RES_SECURITY, [cb{std::move(cb)}] (Error error, const Packet &packet) {
    if (error != Error::Success) {
      cb(error, 0, gsl::span<uint8_t>());
      return;
    }

    if (packet.data.empty()) {
      cb(Error::Malformed, 0, gsl::span<uint8_t>());
      return;
    }

    cb(Error::Success, packet.data[0], gsl::make_span(packet.data).subspan(1));
  });
}

void Protocol::requestSecurityKey(gsl::span<uint8_t> key, RequestSecurityKeyCallback &&cb) {
  std::vector<uint8_t> req(key.size() + 2);
  req[0] = UDS_REQ_SECURITY;
  req[1] = 2;
  std::copy(key.begin(), key.end(), req.begin() + 2);
  request(req, UDS_RES_SECURITY, [cb{std::move(cb)}] (Error error, const Packet &packet) {
    if (error != Error::Success) {
      cb(error, 0);
      return;
    }

    if (packet.data.empty()) {
      cb(Error::Malformed, 0);
      return;
    }

    cb(Error::Success, packet.data[0]);
  });
}

void Protocol::requestReadMemoryAddress(uint32_t address, uint16_t length, RequestMemoryAddressCallback &&cb) {
  std::array<uint8_t, 7> req;
  req[0] = UDS_REQ_READMEM;

  req[1] = (address & 0xFF000000) >> 24;
  req[2] = (address & 0xFF0000) >> 16;
  req[3] = (address & 0xFF00) >> 8;
  req[4] = address & 0xFF;

  req[5] = length >> 8;
  req[6] = length & 0xFF;

  request(req, UDS_RES_READMEM, [cb{std::move(cb)}] (Error error, const Packet &packet) {
    if (error != Error::Success) {
      cb(error, gsl::span<uint8_t>());
      return;
    }

    cb(Error::Success, packet.data);
  });
}
}