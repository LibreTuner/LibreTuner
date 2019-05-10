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

#include <cassert>

#include "checksum.h"
#include "support/util.hpp"

namespace lt {

void Checksum::addModifiable(int offset, int size) {
    modifiable_.emplace_back(offset, size);
}

Checksum::~Checksum() = default;

uint32_t ChecksumBasic::compute(const uint8_t *data, int size, bool *ok) const {
    assert(size >= 0);
    if (size < offset_ + size_) {
        if (ok != nullptr) {
            *ok = false;
        }
        return 0;
    }

    data += offset_;
    size -= offset_;

    uint32_t sum = 0;
    // Add up the big endian int32s
    for (int i = 0; i < size_ / 4; ++i, data += 4) {
        sum += readBE<int32_t>(data, data + size);
    }

    if (ok != nullptr) {
        *ok = true;
    }
    return sum;
}

void ChecksumBasic::correct(uint8_t *data, int size) const {
    assert(size >= 0);
    if (size < offset_ + size_) {
        throw std::runtime_error("checksum region exceeds the rom size.");
    }

    bool foundMod = false;
    int modifiableOffset{0};

    // Find a usable modifiable region
    for (const auto &it : modifiable_) {
        if (it.second >= 4) {
            modifiableOffset = it.first;
            foundMod = true;
            break;
        }
    }
    if (!foundMod) {
        throw std::runtime_error("failed to find a usable modifiable region "
                                 "for checksum correction.");
    }

    // Zero the region
    writeBE<int32_t>(0, &data[offset_ + modifiableOffset], data + size);

    // compute should never fail after the check above
    uint32_t oSum = compute(data, size, nullptr);

    uint32_t val = target_ - oSum;
    writeBE<int32_t>(val, &data[offset_ + modifiableOffset], data + size);

    // Check if the correction was successful
    if (compute(data, size, nullptr) != target_) {
        throw std::runtime_error(
            "checksum does not equal target after correction");
    }
}

void Checksums::correct(uint8_t *data, size_t size) {
    for (const ChecksumPtr &checksum : checksums_) {
        checksum->correct(data, size);
    }
}

} // namespace lt