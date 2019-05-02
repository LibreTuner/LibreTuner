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

#ifndef LT_CHECKSUM_H
#define LT_CHECKSUM_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace lt {

class Checksum {
public:
    Checksum(int offset, int size, uint32_t target)
        : offset_(offset), size_(size), target_(target) {}

    /* Adds a region modifiable for checksum computation */
    void addModifiable(int offset, int size);

    /* Corrects the checksum for the data using modifiable sections. */
    virtual void correct(uint8_t *data, int size) const = 0;

    /* Returns the computed checksum. If length is too small,
     * returns 0 and sets ok to false.*/
    virtual uint32_t compute(const uint8_t *data, int size,
                             bool *ok = nullptr) const = 0;

	virtual ~Checksum();

protected:
    int offset_;
    int size_;
    uint32_t target_;

    std::vector<std::pair<int, int>> modifiable_;
};
using ChecksumPtr = std::unique_ptr<Checksum>;

/* Basic type checksum */
class ChecksumBasic : public Checksum {
public:
    ChecksumBasic(uint32_t offset, uint32_t size, uint32_t target)
        : Checksum(offset, size, target) {}

    uint32_t compute(const uint8_t *data, int size,
                     bool *ok) const override;

    void correct(uint8_t *data, int size) const override;
};

/**
 * Manages ECU checksums
 */
class Checksums {
public:
    /* Adds a basic type checksum */
	inline void add(ChecksumPtr &&checksum) { checksums_.emplace_back(std::move(checksum)); }

    /* Corrects the checksums for the data using modifiable sections.
     * Returns (false, errmsg) on failure and (true, "") on success. */
    void correct(uint8_t *data, size_t size);

private:
    std::vector<ChecksumPtr> checksums_;
};

} // namespace lt

#endif // LT_CHECKSUM_H
