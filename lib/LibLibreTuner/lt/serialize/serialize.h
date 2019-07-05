#ifndef LIBRETUNER_SERIALIZER_H
#define LIBRETUNER_BYTEORDER_H

#include <type_traits>
#include <utility>
#include <vector>

#include "../support/types.h"

namespace lt {

// Arithmetic type serialization
template <typename T, typename S,
          typename = std::enable_if_t<std::is_arithmetic_v<T>>>
void save(S &s, T t) {
    T swapped = endian::convert<T, endian::current, S::Endian>(t);
    s.write(reinterpret_cast<const uint8_t *>(std::addressof(swapped)),
            sizeof(T));
}

template <typename T, typename S, typename Allocator>
void save(S &s, const std::vector<T, Allocator> &vec) {
    s(vec.size());
    s.serialize(vec.data(), vec.size());
}

template <typename A> void archive() {
    // Archive stub - never called
}

// Arithmetic type deserialization
template <typename T, typename D,
          typename = std::enable_if_t<std::is_arithmetic_v<T>>>
T load(D &d, T &t) {
    d.read(reinterpret_cast<const uint8_t *>(std::addressof(t)), sizeof(T));
    t = endian::convert<T, D::Endian, endian::current>(t);
}

// Vector type deserialization
template <typename T, typename D> void load(D &d, std::vector<T> &out) {
    std::size_t size;
    d(size);
    if (size > D::MaxRead) {
        throw std::runtime_error("Size exceeded maximum read");
    }

    out.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        T t;
        d(t);
        out.emplace_back(std::move(t));
    }
}

template <typename Sink, Endianness endianness = Endianness::Little>
class Serializer {
public:
    static constexpr Endianness Endian = endianness;

    template <typename... Args>
    Serializer(Args &&... args) : sink(std::forward<Args>(args)...) {}

    // Serializer for types with external save()
    template <typename T, typename = decltype(save(
                              std::declval<Serializer<Sink, endianness> &>(),
                              std::declval<T>()))>
    inline void operator()(const T &t) {
        save(*this, t);
    }

    // For types that expose an external archive()
    template <typename T, typename = decltype(archive(
                              std::declval<Serializer<Sink, endianness> &>(),
                              std::declval<T>()))>
    inline void archive(const T &t) {
        archive(*this, t);
    }

    // Generic array serialization
    template <typename T> void serialize(const T *array, std::size_t length) {
        serialize(static_cast<uint32_t>(length));
        std::for_each(array, &array[length],
                      [this](const T &t) { serialize(t); });
    }

    // Serialize raw bytes
    inline void write(const uint8_t *d, std::size_t length) {
        sink.write(d, length);
    }

private:
    Sink sink;
};

template<class A, class T>
struct has_external_load {
    template<class AA, class TT>
    static auto test(int) -> decltype(archive(std::declval<AA&>(),
        std::declval<TT&>()), std::true_type());
    static std::false_type test(...);

    static const bool value = std::is_same_v<decltype(test<A, T>(0)), std::true_type()>;
};

template <typename Source, Endianness endianness = Endianness::Little,
          std::size_t max_read = std::numeric_limits<std::size_t>::max()>
class Deserializer {
public:
    static constexpr Endianness Endian = endianness;
    static constexpr std::size_t MaxRead = max_read;

    template <typename... Args>
    Deserializer(Args &&... args) : source(std::forward<Args>(args)...) {}

    // Deserializer for types that define and external load()
    template <
        typename T,
        typename = decltype(load(std::declval<Deserializer<Source, endianness> &>(),
                            std::declval<T>()))>
    inline void operator()(T &t) {
        load(*this, t);
    }

    // For types that expose an external archive()
    template <
        typename T,
        typename = decltype(archive(std::declval<Deserializer<Source, endianness, max_read>&>(),
                                    std::declval<T&>()))>
    inline void archive(T const &t) {
        archive(*this, t);
    }

    // Read raw bytes
    inline void read(uint8_t *d, std::size_t length) { source.read(d, length); }

private:
    Source source;
};

} // namespace lt

#endif // LIBRETUNER_SERIALIZER_H
