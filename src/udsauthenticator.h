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
class UdsAuthenticator {
public:
  using Callback = std::function<void(bool success, const std::string &error)>;
  
  UdsAuthenticator(Callback callback);

  /* UDS Callback */
  void onRecv(const UdsResponse &response);
  
  /* Start authentication */
  void start(const std::string &key, const std::shared_ptr<IsoTpInterface> &isotp, const IsoTpOptions &options);
  // void start(std::shared_ptr<UdsProtocol> uds, const std::string &key);

  uint32_t generateKey(uint32_t parameter, const uint8_t *seed, size_t len);

  enum State {
    STATE_IDLE,
    STATE_SESSION,          // Waiting for response from session control
    STATE_SECURITY_REQUEST, // Waiting for response from security access
                            // requestSeed
    STATE_SECURITY_KEY,     // Waiting for response from security access sendKey
  };

private:
  std::shared_ptr<UdsProtocol> uds_;
  State state_;

  std::string key_;

  void onNegativeResponse(int code);
  void onFail(const std::string &error);

  Callback callback_;
};

#endif // UDSAUTHENTICATOR_H
