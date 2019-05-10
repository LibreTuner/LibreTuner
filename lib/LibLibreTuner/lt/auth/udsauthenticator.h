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

#include "auth.h"
#include "network/uds/uds.h"

#include <string>

/**
 * Handles UDS authentication for flashing & downloading
 */
namespace lt {
namespace auth {

class UdsAuthenticator {
public:
    UdsAuthenticator(network::Uds &uds, Options options);
    /* Start authentication */
    void auth();

    uint32_t generateKey(uint32_t parameter, const uint8_t *seed, size_t size);

private:
    network::Uds &uds_;
    Options options_;

    void do_session();
    void do_request_seed();
    void do_send_key(uint32_t key);
};
} // namespace auth
} // namespace lt

#endif // UDSAUTHENTICATOR_H
