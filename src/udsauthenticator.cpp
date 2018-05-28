/*
 * LibreTuner
 * Copyright (C) 2018  Altenius
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

#include "udsauthenticator.h"

#include <cassert>
#include <sstream>

UdsAuthenticator::UdsAuthenticator(UdsAuthenticator::Callback callback)
    : callback_(callback) {
}

void UdsAuthenticator::start(const std::string &key, const std::shared_ptr<IsoTpInterface> &isotp,
                             const IsoTpOptions &options) {
  key_ = key;
  uds_ = UdsProtocol::create(isotp, options);
  state_ = STATE_SESSION;
  uds_->requestSession(0x87);
}

void UdsAuthenticator::onFail(const std::string &error) {
  callback_(false, error);
}

void UdsAuthenticator::onRecv(const UdsResponse &response) {
  if (response.id() == UDS_RES_NEGATIVE) {
    if (response.length() > 0) {
      onNegativeResponse(response[0]);
    } else {
      onNegativeResponse(0);
    }
    return;
  }

  switch (state_) {
  case STATE_SESSION:
    if (response.id() != UDS_RES_SESSION) {
      onFail("Unepected response. Expected session.");
      return;
    }

    // Request security seed
    state_ = STATE_SECURITY_REQUEST;
    uds_->requestSecuritySeed();

    break;
  case STATE_SECURITY_REQUEST: {
    if (response.id() != UDS_RES_SECURITY) {
      onFail("Unepected response. Expected security.");
      return;
    }

    // TODO: Change to actual length
    if (response.length() < 2) {
      onFail("Unexpected message length in security request response");
      return;
    }

    // Generate key from seed
    uint32_t key = generateKey(
        0xC541A9, reinterpret_cast<const uint8_t *>(response.message() + 1),
        response.length() - 1);

    uint8_t kData[3];
    kData[0] = key & 0xFF;
    kData[1] = (key & 0xFF00) >> 8;
    kData[2] = (key & 0xFF0000) >> 16;

    // Send key
    state_ = STATE_SECURITY_KEY;
    uds_->requestSecurityKey(kData, sizeof(kData));

    break;
  }
  case STATE_SECURITY_KEY:
    if (response.id() != UDS_RES_SECURITY) {
      onFail("Unepected response. Expected security.");
      return;
    }

    // Begin downloading
    state_ = STATE_IDLE;
    callback_(true, "");

    break;
  default:
    // STATE_IDLE
    break;
  }
}

void UdsAuthenticator::onNegativeResponse(int code) {
  std::stringstream ss;
  ss << "Received negative UDS response: 0x" << std::hex << code;
  onFail(ss.str());
}

uint32_t UdsAuthenticator::generateKey(uint32_t parameter, const uint8_t *seed,
                                       size_t len) {
  std::vector<uint8_t> nseed(seed, seed + len);
  nseed.insert(nseed.end(), key_.begin(), key_.end());

  for (uint8_t c : nseed) {
    for (int r = 8; r > 0; --r) {
      uint8_t s = (c & 1) ^ (parameter & 1);
      uint32_t m = 0;
      if (s != 0) {
        parameter |= 0x1000000;
        m = 0x109028;
      }

      c >>= 1;
      parameter >>= 1;
      uint32_t p3 = parameter & 0xFFEF6FD7;
      parameter ^= m;
      parameter &= 0x109028;

      parameter |= p3;

      parameter &= 0xFFFFFF;
    }
  }

  uint32_t res = (parameter >> 4) & 0xFF;
  res |= (((parameter >> 20) & 0xFF) + ((parameter >> 8) & 0xF0)) << 8;
  res |= (((parameter << 4) & 0xFF) + ((parameter >> 16) & 0xF)) << 16;

  return res;
}
