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

#include "checksummanager.h"
#include "util.hpp"

void Checksum::addModifiable(uint32_t offset, uint32_t size) {
  modifiable_.emplace_back(offset, size);
}

uint32_t ChecksumBasic::compute(gsl::span<const uint8_t> data,
                                bool *ok) const {
  if (data.size() < offset_ + size_) {
    if (ok != nullptr) {
      *ok = false;
    }
    return 0;
  }

  data = data.subspan(offset_);

  uint32_t sum = 0;
  // Add up the big endian int32s
  for (int i = 0; i < size_ / 4; ++i, data = data.subspan(4)) {
    sum += readBE<int32_t>(data);
  }

  if (ok != nullptr) {
    *ok = true;
  }
  return sum;
}

std::pair<bool, std::string> ChecksumBasic::correct(gsl::span<uint8_t> data) const {
  if (data.size() < offset_ + size_) {
    return std::make_pair<bool, std::string>(
        false, "Checksum region exceeds the rom size.");
  }

  bool foundMod = false;
  uint32_t modifiableOffset;

  // Find a usable modifiable region
  for (const auto &it : modifiable_) {
    if (it.second >= 4) {
      modifiableOffset = it.first;
      foundMod = true;
      break;
    }
  }
  if (!foundMod) {
    return std::make_pair<bool, std::string>(
        false,
        "Failed to find a usable modifiable region for checksum correction.");
  }

  // Zero the region
  writeBE<int32_t>(0, data.subspan(offset_ + modifiableOffset));

  // compute should never fail after the check above
  uint32_t oSum = compute(data);

  uint32_t val = target_ - oSum;
  writeBE<int32_t>(val, data.subspan(offset_ + modifiableOffset));

  // Check if the correction was successful
  if (compute(data) != target_) {
    return std::make_pair<bool, std::string>(
        false, "Checksum does not equal target after correction");
  }

  return std::make_pair<bool, std::string>(true, std::string());
}

std::pair<bool, std::string> ChecksumManager::correct(gsl::span<uint8_t> data) {
  for (const ChecksumPtr &checksum : checksums_) {
    auto res = checksum->correct(data);
    if (!res.first) {
      return res;
    }
  }
  return std::make_pair<bool, std::string>(true, std::string());
}

ChecksumBasic *ChecksumManager::addBasic(uint32_t offset, uint32_t size,
                                         uint32_t target) {
  ChecksumPtr checksum = std::make_shared<ChecksumBasic>(offset, size, target);

  checksums_.push_back(checksum);
  return reinterpret_cast<ChecksumBasic *>(checksum.get());
}
