#ifndef LT_ENDIANNESS_H
#define LT_ENDIANNESS_H

#include <cstdint>
#include <type_traits>
#include <algorithm>

namespace lt {
enum class Endianness { Big, Little };

namespace endian {
namespace detail {
    // https://stackoverflow.com/questions/1583791/constexpr-and-endianness
    constexpr uint32_t test = 0x01020304;
    constexpr uint8_t magic = static_cast<const uint8_t&>(test);
    constexpr bool isLittle = magic == 0x04;
    constexpr bool isBig = magic == 0x01;
}

constexpr bool isBig = detail::isBig;
constexpr bool isLittle = detail::isLittle;
constexpr Endianness current = (isLittle) ? Endianness::Little : Endianness::Big;

template<typename T>
T swap(T t) {
    static_assert(std::is_arithmetic_v<T>, "Type must be arithmetic type");
    constexpr std::size_t type_size = sizeof(T);

    auto *raw = reinterpret_cast<uint8_t*>(&t);
    std::reverse(raw, raw + type_size);
    return *reinterpret_cast<T*>(raw);
}

template<typename T, Endianness from, Endianness to>
T convert(T t) {
    if constexpr (from == to) {
        return t;
    }

    return swap(t);
}

// Converts from native format to big endian
template<typename T>
T toBig(T t) {
    return convert<T, current, Endianness::Big>(t);
}

// Converts from native format to little endian
template<typename T>
T toLittle(T t) {
    return convert<T, current, Endianness::Little>(t);
}

// Converts to native format from big endian
template<typename T>
T fromBig(T t) {
    return convert<T, Endianness::Big, current>(t);
}

// Converts to native format from little endian
template<typename T>
T fromLittle(T t) {
    return convert<T, Endianness::Little, current>(t);
}

} // namespace endian

} // namespace lt

#endif //LT_ENDIANNESS_H
