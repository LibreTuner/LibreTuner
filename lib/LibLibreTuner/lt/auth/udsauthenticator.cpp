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
namespace lt {
namespace auth {

UdsAuthenticator::UdsAuthenticator(network::Uds &uds, auth::Options options)
    : uds_(uds), options_(std::move(options)) {}

void UdsAuthenticator::auth() { do_session(); }

void UdsAuthenticator::do_session() {
    uds_.requestSession(options_.session);
    do_request_seed();
}

void UdsAuthenticator::do_request_seed() {
    std::vector<uint8_t> seed = uds_.requestSecuritySeed();

    // Generate key from seed
    uint32_t key = generateKey(0xC541A9, seed.data(), seed.size());
    do_send_key(key);
}

void UdsAuthenticator::do_send_key(uint32_t key) {
    uint8_t kData[3];
    kData[0] = key & 0xFF;
    kData[1] = (key & 0xFF00) >> 8;
    kData[2] = (key & 0xFF0000) >> 16;

    uds_.requestSecurityKey(kData, 3);
}

uint32_t UdsAuthenticator::generateKey(uint32_t parameter, const uint8_t *seed,
                                       size_t size) {
    std::vector<uint8_t> nseed(seed, seed + size);
    nseed.insert(nseed.end(), options_.key.begin(), options_.key.end());

    // This is Mazda's key generation algorithm reverse engineered from a
    // Mazda 6 MPS ROM. Internally, the ECU uses a timer/counter for the seed
    // generation

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

} // namespace auth
} // namespace lt
