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

namespace uds {
Authenticator::Authenticator(Callback &&callback)
    : callback_(std::move(callback)) {}

bool Authenticator::checkError(Error error) {
  if (error != Error::Success) {
    onFail(strError(error));
    return false;
  }
  return true;
}

void Authenticator::auth(const std::string &key,
                         std::shared_ptr<uds::Protocol> uds,
                         uint8_t sessionType) {
  key_ = key;
  uds_ = std::move(uds);

  do_session(sessionType);
}

void Authenticator::do_session(uint8_t sessionType) {
  uds_->requestSession(
      sessionType, [this](Error error, uint8_t type, gsl::span<const uint8_t>) {
        if (!checkError(error)) {
          return;
        }

        do_request_seed();
      });
}

void Authenticator::do_request_seed() {
  uds_->requestSecuritySeed(
      [this](Error error, uint8_t type, gsl::span<const uint8_t> seed) {
        if (!checkError(error)) {
          return;
        }

        // Generate key from seed
        uint32_t key = generateKey(0xC541A9, seed);

        do_send_key(key);
      });
}

void Authenticator::do_send_key(uint32_t key) {
  uint8_t kData[3];
  kData[0] = key & 0xFF;
  kData[1] = (key & 0xFF00) >> 8;
  kData[2] = (key & 0xFF0000) >> 16;

  uds_->requestSecurityKey(kData, [this](Error error, uint8_t type) {
    if (!checkError(error)) {
      return;
    }

    callback_(true, "");
  });
}

void Authenticator::onFail(const std::string &error) {
  callback_(false, error);
}

uint32_t Authenticator::generateKey(uint32_t parameter,
                                    gsl::span<const uint8_t> seed) {
  std::vector<uint8_t> nseed(seed.begin(), seed.end());
  nseed.insert(nseed.end(), key_.begin(), key_.end());

  // This is Mazda's key generation algorithm reverse engineered from a
  // Mazdaspeed6 ROM. Internally, the ECU uses a timer/counter for the seed

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
  res |= (((parameter << 4) & 0xFF) + ((parameter >> 16) & 0x0F)) << 16;

  return res;
}
} // namespace uds