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

#ifndef LT_UTIL_H
#define LT_UTIL_H

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>

#include "endianness.h"

namespace lt
{
template <typename T, int Size> class SConverter
{
};

template <typename T> class SConverter<T, 1>
{
public:
    template <class InputIt> static T readBE(InputIt begin)
    {
        static_assert(
            sizeof(T) == 1,
            "type parameter of this class must have a size of 1 byte");
        uint8_t n = *begin;
        return *reinterpret_cast<T *>(&n);
    }

    template <class OutputIt> static void writeBE(T t, OutputIt begin)
    {
        static_assert(
            sizeof(T) == 1,
            "type parameter of this class must have a size of 1 byte");
        *begin = *reinterpret_cast<uint8_t *>(&t);
    }

    template <class OutputIt> static void writeLE(T t, OutputIt begin)
    {
        static_assert(
            sizeof(T) == 1,
            "type parameter of this class must have a size of 1 byte");
        *begin = *reinterpret_cast<uint8_t *>(&t);
    }

    template <class InputIt> static T readLE(InputIt begin)
    {
        static_assert(
            sizeof(T) == 1,
            "type parameter of this class must have a size of 1 byte");
        uint8_t n = *begin;
        return *reinterpret_cast<T *>(&n);
    }
};

template <typename T> class SConverter<T, 2>
{
public:
    template <class InputIt> static T readBE(InputIt begin)
    {
        static_assert(
            sizeof(T) == 2,
            "type parameter of this class must have a size of 2 bytes");
        uint16_t n = (*begin << 8) | *(begin + 1);
        return *reinterpret_cast<T *>(&n);
    }

    template <class OutputIt> static void writeBE(T t, OutputIt begin)
    {
        static_assert(
            sizeof(T) == 2,
            "type parameter of this class must have a size of 2 bytes");
        uint16_t n = *reinterpret_cast<uint16_t *>(&t);
        *begin = n >> 8;
        *(begin + 1) = n & 0xFF;
    }

    template <class OutputIt> static void writeLE(T t, OutputIt begin)
    {
        static_assert(
            sizeof(T) == 2,
            "type parameter of this class must have a size of 2 bytes");
        uint16_t n = *reinterpret_cast<uint16_t *>(&t);
        *(begin + 1) = n >> 8;
        *begin = n & 0xFF;
    }

    template <class InputIt> static T readLE(InputIt begin)
    {
        static_assert(
            sizeof(T) == 2,
            "type parameter of this class must have a size of 2 bytes");
        uint16_t n = (*(begin + 1) << 8) | *begin;
        return *reinterpret_cast<T *>(&n);
    }
};

template <typename T> class SConverter<T, 4>
{
public:
    template <class InputIt> static T readBE(InputIt begin)
    {
        static_assert(
            sizeof(T) == 4,
            "type parameter of this class must have a size of 4 bytes");
        uint32_t n = ((uint32_t)(*begin) << 24) | (*(begin + 1) << 16) |
                     (*(begin + 2) << 8) | *(begin + 3);
        return *reinterpret_cast<T *>(&n);
    }

    template <class OutputIt> static void writeBE(T t, OutputIt begin)
    {
        static_assert(
            sizeof(T) == 4,
            "type parameter of this class must have a size of 4 bytes");
        uint32_t n = *reinterpret_cast<uint32_t *>(&t);
        *begin = n >> 24;
        *(begin + 1) = (n >> 16) & 0xFF;
        *(begin + 2) = (n >> 8) & 0xFF;
        *(begin + 3) = n & 0xFF;
    }

    template <class OutputIt> static void writeLE(T t, OutputIt begin)
    {
        static_assert(
            sizeof(T) == 4,
            "type parameter of this class must have a size of 4 bytes");
        uint32_t n = *reinterpret_cast<uint32_t *>(&t);
        *(begin + 3) = n >> 24;
        *(begin + 2) = (n >> 16) & 0xFF;
        *(begin + 1) = (n >> 8) & 0xFF;
        *begin = n & 0xFF;
    }

    template <class InputIt> static T readLE(InputIt begin)
    {
        static_assert(
            sizeof(T) == 4,
            "type parameter of this class must have a size of 4 bytes");
        uint32_t n = (*(begin + 3) << 24) | (*(begin + 2) << 16) |
                     (*(begin + 1) << 8) | *begin;
        return *reinterpret_cast<T *>(&n);
    }
};

template <typename T, class InputIt> static T readBE(InputIt begin, InputIt end)
{
    static_assert(sizeof(*std::declval<InputIt>()) == 1,
                  "Output iterator must be byte iterator");

    auto const max = std::distance(begin, end);
    assert(max >= 0);

    if (static_cast<std::size_t>(max) < sizeof(T))
    {
        throw std::length_error("size of data is less than size of type");
    }
    return SConverter<T, sizeof(T)>::readBE(std::forward<InputIt>(begin));
}

template <typename T, class InputIt> static T readLE(InputIt begin, InputIt end)
{
    static_assert(sizeof(*std::declval<InputIt>()) == 1,
                  "Output iterator must be byte iterator");

    auto const max = std::distance(begin, end);
    assert(max >= 0);

    if (static_cast<std::size_t>(max) < sizeof(T))
    {
        throw std::length_error("size of data is less than size of type");
    }
    return SConverter<T, sizeof(T)>::readLE(std::forward<InputIt>(begin));
}

// reads an array of values
template <typename T, class InputIt, class OutputIt>
static void readBE(InputIt begin, InputIt end, OutputIt out)
{
    for (; begin < end; begin += sizeof(T))
    {
        *out++ = SConverter<T, sizeof(T)>::readBE(begin);
    }
}

template <typename T, class InputIt, class OutputIt>
static void readLE(InputIt begin, InputIt end, OutputIt out)
{
    for (; begin < end; begin += sizeof(T))
    {
        *out++ = SConverter<T, sizeof(T)>::readLE(begin);
    }
}

template <typename T, class OutputIt>
static void writeBE(T t, OutputIt begin, OutputIt end)
{
    static_assert(sizeof(*std::declval<OutputIt>()) == 1,
                  "Output iterator must be byte iterator");

    auto const max = std::distance(begin, end);
    assert(max >= 0);

    if (static_cast<std::size_t>(max) < sizeof(T))
    {
        throw std::length_error("size of data is less than size of type");
    }
    return SConverter<T, sizeof(T)>::writeBE(t, std::forward<OutputIt>(begin));
}

template <typename T, class OutputIt>
static void writeLE(T t, OutputIt begin, OutputIt end)
{
    static_assert(sizeof(*std::declval<OutputIt>()) == 1,
                  "Output iterator must be byte iterator");

    auto const max = std::distance(begin, end);
    assert(max >= 0);

    if (static_cast<std::size_t>(max) < sizeof(T))
    {
        throw std::length_error("size of data is less than size of type");
    }
    return SConverter<T, sizeof(T)>::writeLE(t, std::forward<OutputIt>(begin));
}

// Writes an array of values
template <typename T, class OutputIt, class InputIt>
static void writeBE(InputIt values, InputIt end, OutputIt out)
{
    static_assert(sizeof(*std::declval<OutputIt>()) == 1,
                  "Output iterator must be byte iterator");
    for (; values < end; values++)
    {
        SConverter<T, sizeof(T)>::writeBE(*values, out);
        out += sizeof(T);
    }
}

template <typename T, class OutputIt, class InputIt>
static void writeLE(InputIt values, InputIt end, OutputIt out)
{
    static_assert(sizeof(*std::declval<OutputIt>()) == 1,
                  "Output iterator must be byte iterator");
    for (; values < end; values++)
    {
        SConverter<T, sizeof(T)>::writeLE(*values, out);
        out += sizeof(T);
    }
}

inline void remove_whitespace(std::string & string)
{
    string.erase(std::remove_if(string.begin(), string.end(),
                                [](unsigned char ch) {
                                    return std::isspace(
                                        static_cast<unsigned char>(ch));
                                }),
                 string.end());
}

inline void trim(std::string & string)
{
    // Trim beginning
    string.erase(string.begin(), std::find_if_not(string.begin(), string.end(),
                                                  [](unsigned char c) {
                                                      return std::isspace(c);
                                                  }));
    // Trim end
    string.erase(
        std::find_if_not(string.rbegin(), string.rend(),
                         [](unsigned char c) { return std::isspace(c); })
            .base(),
        string.end());
}

inline void lowercase_string(std::string & string)
{
    std::transform(string.begin(), string.end(), string.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}

template <typename T, typename It, Endianness endianness>
std::vector<T> fromBytes_(It && begin, It && end)
{
    static_assert(sizeof(*std::declval<It>()) == 1,
                  "Iterator type must be one byte long");
    std::vector<T> entries;
    for (;;)
    {
        if (begin == end)
            break;
        T val;
        uint8_t * repr = reinterpret_cast<uint8_t *>(&val);
        // Unroll this loop, compiler!
        for (std::size_t i = 0; i < sizeof(T); ++i)
        {
            if (std::next(begin, 1) == end)
                break;
            repr[i] = *begin;
        }
        // Convert endianness and emplace
        entries.emplace_back(
            endian::convert<T, endianness, endian::current>(val));
    }
    return entries;
}

/* Converts the representation of an array in bytes into an array of the
 * type. */
template <typename T, typename It>
std::vector<T> fromBytes(It && begin, It && end, Endianness endianness)
{
    switch (endianness)
    {
    case Endianness::Big:
        return fromBytes_<T, It, Endianness::Big>(std::forward<It>(begin),
                                                  std::forward<It>(end));
    case Endianness::Little:
        return fromBytes_<T, It, Endianness::Little>(std::forward<It>(begin),
                                                     std::forward<It>(end));
    default:
        return std::vector<T>();
    }
}

} // namespace lt

template <class T> struct make_shared_enabler : public T
{
    template <typename... Args>
    explicit make_shared_enabler(Args &&... args)
        : T(std::forward<Args>(args)...)
    {
    }
};

#endif // LT_UTIL_H
