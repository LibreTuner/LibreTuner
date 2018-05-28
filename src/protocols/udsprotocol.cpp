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
#include "isotpinterface.h"

#include <cassert>

UdsResponse::UdsResponse(uint8_t id, const uint8_t *message, size_t length)
    : responseId_(id), message_(message, message + length), error_(UdsError::Success) {}

UdsResponse::UdsResponse(IsoTpError error) : error_(UdsError::IsoTp), isotpError_(error) {
}

std::string UdsResponse::strError() const
{
  switch (error_) {
    case UdsError::Success:
      return "success";
    case UdsError::IsoTp:
      return IsoTpInterface::strError(isotpError_);
    case UdsError::Timeout:
      return "timed out";
    case UdsError::BlankResponse:
      return "received a blank response";
    case UdsError::Can:
      return "CAN error";
    default:
      return "unknown";
  }
  return "you should never see this";
}


void UdsResponse::setMessage(const uint8_t *message, size_t length) {
  message_.assign(message, message + length);
}

class IsoTpUdsInterface : public UdsProtocol {
public:
  IsoTpUdsInterface(const std::shared_ptr<IsoTpInterface> &isotp,
                    const IsoTpOptions &options);

  boost::future<UdsResponse> request(const uint8_t *message, size_t length) override;
private:
  std::shared_ptr<IsoTpInterface> isotp_;
  IsoTpOptions options_;
};

boost::future<UdsResponse> IsoTpUdsInterface::request(const uint8_t *message, size_t length) {
  std::shared_ptr<boost::promise<UdsResponse>> promise = std::make_shared<boost::promise<UdsResponse>>();
  isotp_->request(message, length, options_).then([promise, this](boost::future<IsoTpInterface::Response> f) {
    IsoTpInterface::Response res = f.get();
    if (!res.success()) {
      IsoTpInterface::Response::Error error = res.error();
      switch (error) {
        case IsoTpInterface::Response::Error::CanError:
          promise->set_value(UdsResponse(UdsError::Can));
          break;
        case IsoTpInterface::Response::Error::IsoTpError:
          promise->set_value(UdsResponse(res.isotpError()));
          break;
        case IsoTpInterface::Response::Error::Timeout:
          promise->set_value(UdsResponse(UdsError::Timeout));
          break;
        default:
          promise->set_value(UdsResponse(UdsError::Unknown));
          break;
      }
      return;
    }
    
    IsoTpMessage &msg = res.message();
    if (msg.length() == 0) {
      promise->set_value(UdsResponse(UdsError::BlankResponse));
    }
    
    promise->set_value(UdsResponse(msg.message()[0], msg.message() + 1, msg.length() - 1));
  });
  return promise->get_future();
}

IsoTpUdsInterface::IsoTpUdsInterface(const std::shared_ptr<IsoTpInterface> &isotp,
                                     const IsoTpOptions &options)
    : isotp_(isotp), options_(options) {
      
}



std::shared_ptr<UdsProtocol>
UdsProtocol::create(const std::shared_ptr<IsoTpInterface> &isotp,
                    const IsoTpOptions &options) {
  return std::make_shared<IsoTpUdsInterface>(isotp, options);
}

boost::future<UdsResponse> UdsProtocol::requestSession(uint8_t type) {
  uint8_t req[] = {UDS_REQ_SESSION, type};
  return request(req, sizeof(req));
}

boost::future<UdsResponse> UdsProtocol::requestSecuritySeed() {
  uint8_t req[] = {UDS_REQ_SECURITY, 1};
  return request(req, sizeof(req));
}

boost::future<UdsResponse> UdsProtocol::requestSecurityKey(const uint8_t *key, uint8_t length) {
  std::vector<uint8_t> req(length + 2);
  req[0] = UDS_REQ_SECURITY;
  req[1] = 2;
  memcpy(req.data() + 2, key, length);
  return request(req.data(), length + 2);
}

boost::future<UdsResponse> UdsProtocol::requestReadMemoryAddress(const uint32_t address,
                                           uint16_t length) {
  uint8_t req[7];
  req[0] = UDS_REQ_READMEM;

  req[1] = (address & 0xFF000000) >> 24;
  req[2] = (address & 0xFF0000) >> 16;
  req[3] = (address & 0xFF00) >> 8;
  req[4] = address & 0xFF;

  req[5] = length >> 8;
  req[6] = length & 0xFF;

  return request(req, sizeof(req));
}
