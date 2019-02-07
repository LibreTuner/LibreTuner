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
#include <functional>
#include <string>

/**
 * Handles UDS authentication for flashing & downloading
 */
namespace auth {
struct Options {
    std::string key;
    uint8_t session = 0x87;
};

class UdsAuthenticator {
public:
    /* Start authentication */
    void auth(uds::Protocol &uds, Options options);

    uint32_t generateKey(uint32_t parameter, const uint8_t *seed, size_t size);

private:
    uds::Protocol *uds_;
    std::string key_;

    void do_session(uint8_t sessionType);
    void do_request_seed();
    void do_send_key(uint32_t key);
};
} // namespace uds

#endif // UDSAUTHENTICATOR_H
