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

#ifndef UDSAUTHENTICATOR_H
#define UDSAUTHENTICATOR_H

#include "protocols/udsprotocol.h"
#include <string>
#include <functional>

/**
 * Handles UDS authentication for flashing & downloading
 */
namespace uds {
class Authenticator {
public:
  using Callback = std::function<void(bool success, const std::string &error)>;

  explicit Authenticator(Callback &&callback);

  /* Start authentication */
  void auth(const std::string &key, std::shared_ptr<uds::Protocol> uds, uint8_t sessionType = 0x87);
  // void start(std::shared_ptr<UdsProtocol> uds, const std::string &key);

  uint32_t generateKey(uint32_t parameter, gsl::span<const uint8_t> seed);
private:
  std::shared_ptr<uds::Protocol> uds_;
  std::string key_;

  void do_session(uint8_t sessionType);
  void do_request_seed();
  void do_send_key(uint32_t key);

  void onNegativeResponse(int code);
  void onFail(const std::string &error);
  bool checkError(Error error);

  Callback callback_;
};
}

#endif // UDSAUTHENTICATOR_H
