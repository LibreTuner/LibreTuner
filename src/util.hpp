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

#ifndef UTIL_H
#define UTIL_H

#include <gsl/gsl>

namespace util {
template <typename T, int Size> class SConverter {};

template <typename T> class SConverter<T, 1> {
public:
    static T readBE(gsl::span<const uint8_t> data) {
        static_assert(
            sizeof(T) == 1,
            "type parameter of this class must have a size of 1 byte");
        uint8_t n = data[0];
        return *reinterpret_cast<T *>(&n);
    }

    static void writeBE(T t, gsl::span<uint8_t> &data) {
        static_assert(
            sizeof(T) == 1,
            "type parameter of this class must have a size of 1 byte");
        data[0] = *reinterpret_cast<uint8_t *>(&t);
    }

    static void writeLE(T t, gsl::span<uint8_t> &data) {
        static_assert(
            sizeof(T) == 1,
            "type parameter of this class must have a size of 1 byte");
        data[0] = *reinterpret_cast<uint8_t *>(&t);
    }

    static T readLE(gsl::span<const uint8_t> data) {
        static_assert(
            sizeof(T) == 1,
            "type parameter of this class must have a size of 1 byte");
        uint8_t n = data[0];
        return *reinterpret_cast<T *>(&n);
    }
};

template <typename T> class SConverter<T, 2> {
public:
    static T readBE(gsl::span<const uint8_t> data) {
        static_assert(
            sizeof(T) == 2,
            "type parameter of this class must have a size of 2 bytes");
        uint16_t n = (data[0] << 8) | data[1];
        return *reinterpret_cast<T *>(&n);
    }

    static void writeBE(T t, gsl::span<uint8_t> &data) {
        static_assert(
            sizeof(T) == 2,
            "type parameter of this class must have a size of 2 bytes");
        uint16_t n = *reinterpret_cast<uint16_t *>(&t);
        data[0] = n >> 8;
        data[1] = n & 0xFF;
    }

    static void writeLE(T t, gsl::span<uint8_t> &data) {
        static_assert(
            sizeof(T) == 2,
            "type parameter of this class must have a size of 2 bytes");
        uint16_t n = *reinterpret_cast<uint16_t *>(&t);
        data[1] = n >> 8;
        data[0] = n & 0xFF;
    }

    static T readLE(gsl::span<const uint8_t> data) {
        static_assert(
            sizeof(T) == 2,
            "type parameter of this class must have a size of 2 bytes");
        uint16_t n = (data[1] << 8) | data[0];
        return *reinterpret_cast<T *>(&n);
    }
};

template <typename T> class SConverter<T, 4> {
public:
    static T readBE(gsl::span<const uint8_t> data) {
        static_assert(
            sizeof(T) == 4,
            "type parameter of this class must have a size of 4 bytes");
        uint32_t n =
            (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
        return *reinterpret_cast<T *>(&n);
    }

    static void writeBE(T t, gsl::span<uint8_t> &data) {
        static_assert(
            sizeof(T) == 4,
            "type parameter of this class must have a size of 4 bytes");
        uint32_t n = *reinterpret_cast<uint32_t *>(&t);
        data[0] = n >> 24;
        data[1] = (n >> 16) & 0xFF;
        data[2] = (n >> 8) & 0xFF;
        data[3] = n & 0xFF;
    }

    static void writeLE(T t, gsl::span<uint8_t> &data) {
        static_assert(
            sizeof(T) == 4,
            "type parameter of this class must have a size of 4 bytes");
        uint32_t n = *reinterpret_cast<uint32_t *>(&t);
        data[3] = n >> 24;
        data[2] = (n >> 16) & 0xFF;
        data[1] = (n >> 8) & 0xFF;
        data[0] = n & 0xFF;
    }

    static T readLE(gsl::span<const uint8_t> data) {
        static_assert(
            sizeof(T) == 4,
            "type parameter of this class must have a size of 4 bytes");
        uint32_t n =
            (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
        return *reinterpret_cast<T *>(&n);
    }
};
} // namespace util

template <typename T> static T readBE(gsl::span<const uint8_t> data) {
    if (data.size() < sizeof(T)) {
        throw std::length_error("size of data is less than size of type");
    }
    return util::SConverter<T, sizeof(T)>::readBE(data);
}

template <typename T> static T readLE(gsl::span<const uint8_t> data) {
    if (data.size() < sizeof(T)) {
        throw std::length_error("size of data is less than size of type");
    }
    return util::SConverter<T, sizeof(T)>::readLE(data);
}

template <typename T> static void writeBE(T t, gsl::span<uint8_t> data) {
    if (data.size() < sizeof(T)) {
        throw std::length_error("size of data is less than size of type");
    }
    return util::SConverter<T, sizeof(T)>::writeBE(t, data);
}

template <typename T> static void writeLE(T t, gsl::span<uint8_t> data) {
    if (data.size() < sizeof(T)) {
        throw std::length_error("size of data is less than size of type");
    }
    return util::SConverter<T, sizeof(T)>::writeLE(t, data);
}

template <class T> struct make_shared_enabler : public T {
    template <typename... Args>
    explicit make_shared_enabler(Args &&... args)
        : T(std::forward<Args>(args)...) {}
};

#endif
