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

class IsoTpInterface : public Protocol, public std::enable_shared_from_this<IsoTpInterface> {
public:
  class Request {
  public:
    explicit Request(std::shared_ptr<isotp::Protocol> isotp);
    void request(gsl::span<uint8_t> data, uint8_t expectedId, Callback &&cb);

    bool active() const { return active_; }

  private:
    uint8_t expectedId_{};
    Callback cb_;
    bool active_ = false;
    std::shared_ptr<isotp::Protocol> isotp_;

    void do_recv();
  };

  explicit IsoTpInterface(std::shared_ptr<isotp::Protocol> isotp);

  void request(gsl::span<uint8_t>, uint8_t expectedId, Callback &&cb) override;

private:
  Request request_;
  std::shared_ptr<IsoTpInterface> self_;
};

IsoTpInterface::Request::Request(std::shared_ptr<isotp::Protocol> isotp)
    : isotp_(std::move(isotp)) {}

void IsoTpInterface::Request::request(gsl::span<uint8_t> data,
                                      uint8_t expectedId,
                                      Protocol::Callback &&cb) {
  expectedId_ = expectedId;
  cb_ = std::move(cb);
  do_recv();
  isotp_->send(isotp::Packet(data), [this](isotp::Error error) {
      Logger::debug("Sent request");
    if (error != isotp::Error::Success) {
      cb_(Error::IsoTp, Packet());
    }
  });
}

void IsoTpInterface::Request::do_recv() {
  isotp_->recvPacketAsync([this](isotp::Error error, isotp::Packet &&packet) {
    if (error != isotp::Error::Success) {
      switch (error) {
      case isotp::Error::Timeout:
        cb_(Error::Timeout, Packet{});
        break;
      case isotp::Error::Consec:
        cb_(Error::Consec, Packet{});
        break;
      default:
        cb_(Error::IsoTp, Packet{});
        break;
      }
      return;
    }

    Packet res;
    res.id = packet.next();
    res.data = packet.next(packet.remaining());
    if (res.data.empty()) {
      cb_(Error::BlankResponse, res);
      return;
    }

    if (res.id == UDS_RES_NEGATIVE) {
      if (res.data.size() >= 2) {
        uint8_t code = res.data[1];
        if (code == UDS_NRES_RCRRP) {
          // Response pending
          do_recv();
          return;
        }
      }
      cb_(Error::Negative, res);
      return;
    }

    if (res.id != expectedId_) {
      cb_(Error::UnexpectedResponse, Packet{});
      return;
    }
    cb_(Error::Success, res);
  });
}

void IsoTpInterface::request(gsl::span<uint8_t> data, uint8_t expectedId,
                             Callback &&cb) {
  if (request_.active()) {
    throw std::runtime_error("a UDS request is already in progress");
  }
  self_ = shared_from_this();
  request_.request(data, expectedId, [this, cb{std::move(cb)}] (Error error, const Packet &p) {
      self_.reset();
      cb(error, p);
  });
}

IsoTpInterface::IsoTpInterface(std::shared_ptr<isotp::Protocol> isotp)
    : request_(std::move(isotp)) {}

std::shared_ptr<Protocol>
Protocol::create(std::shared_ptr<isotp::Protocol> isotp) {
  return std::make_shared<IsoTpInterface>(std::move(isotp));
}

void Protocol::requestSession(uint8_t type, RequestSessionCallback &&cb) {
  std::array<uint8_t, 2> req = {UDS_REQ_SESSION, type};
  request(req, UDS_RES_SESSION,
          [cb{std::move(cb)}](Error error, const Packet &packet) {
            if (error != Error::Success) {
              cb(error, 0, gsl::span<uint8_t>());
              return;
            }

            if (packet.data.empty()) {
              cb(Error::Malformed, 0, gsl::span<uint8_t>());
              return;
            }

            cb(Error::Success, packet.data[0],
               gsl::make_span(packet.data).subspan(1));
          });
}

void Protocol::requestSecuritySeed(RequestSecuritySeedCallback &&cb) {
  std::array<uint8_t, 2> req = {UDS_REQ_SECURITY, 1};
  return request(req, UDS_RES_SECURITY,
                 [cb{std::move(cb)}](Error error, const Packet &packet) {
                   if (error != Error::Success) {
                     cb(error, 0, gsl::span<uint8_t>());
                     return;
                   }

                   if (packet.data.empty()) {
                     cb(Error::Malformed, 0, gsl::span<uint8_t>());
                     return;
                   }

                   cb(Error::Success, packet.data[0],
                      gsl::make_span(packet.data).subspan(1));
                 });
}

void Protocol::requestSecurityKey(gsl::span<uint8_t> key,
                                  RequestSecurityKeyCallback &&cb) {
  std::vector<uint8_t> req(key.size() + 2);
  req[0] = UDS_REQ_SECURITY;
  req[1] = 2;
  std::copy(key.begin(), key.end(), req.begin() + 2);
  request(req, UDS_RES_SECURITY,
          [cb{std::move(cb)}](Error error, const Packet &packet) {
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

void Protocol::requestReadMemoryAddress(uint32_t address, uint16_t length,
                                        RequestMemoryAddressCallback &&cb) {
  std::array<uint8_t, 7> req{};
  req[0] = UDS_REQ_READMEM;

  req[1] = (address & 0xFF000000) >> 24;
  req[2] = (address & 0xFF0000) >> 16;
  req[3] = (address & 0xFF00) >> 8;
  req[4] = address & 0xFF;

  req[5] = length >> 8;
  req[6] = length & 0xFF;

  request(req, UDS_RES_READMEM,
          [cb{std::move(cb)}](Error error, const Packet &packet) {
            if (error != Error::Success) {
              cb(error, gsl::span<uint8_t>());
              return;
            }

            cb(Error::Success, packet.data);
  });
}

} // namespace uds
