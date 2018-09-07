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

#ifndef CHECKSUMMANAGER_H
#define CHECKSUMMANAGER_H

#include <cstdint>
#include <gsl/span>
#include <memory>
#include <string>
#include <vector>

class Checksum;
typedef std::shared_ptr<Checksum> ChecksumPtr;

class Checksum {
public:
    Checksum(uint32_t offset, uint32_t size, uint32_t target)
        : offset_(offset), size_(size), target_(target) {}

    /* Adds a region modifiable for checksum computation */
    void addModifiable(uint32_t offset, uint32_t size);

    /* Corrects the checksum for the data using modifiable sections. */
    virtual void correct(gsl::span<uint8_t> data) const = 0;

    /* Returns the computed checksum. If length is too small,
     * returns 0 and sets ok to false.*/
    virtual uint32_t compute(gsl::span<const uint8_t> data,
                             bool *ok = nullptr) const = 0;

protected:
    uint32_t offset_;
    uint32_t size_;
    uint32_t target_;

    std::vector<std::pair<uint32_t, uint32_t>> modifiable_;
};

/* Basic type checksum */
class ChecksumBasic : public Checksum {
public:
    ChecksumBasic(uint32_t offset, uint32_t size, uint32_t target)
        : Checksum(offset, size, target) {}

    uint32_t compute(gsl::span<const uint8_t> data,
                     bool *ok = nullptr) const override;

    void correct(gsl::span<uint8_t> data) const override;
};

/**
 * Manages ECU checksums
 */
class ChecksumManager {
public:
    /* Adds a basic type checksum */
    ChecksumBasic *addBasic(uint32_t offset, uint32_t size, uint32_t target);

    /* Corrects the checksums for the data using modifiable sections.
     * Returns (false, errmsg) on failure and (true, "") on success. */
    void correct(gsl::span<uint8_t> data);

private:
    std::vector<ChecksumPtr> checksums_;
};

#endif // CHECKSUMMANAGER_H
