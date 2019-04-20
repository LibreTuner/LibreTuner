#ifndef LIBRETUNER_SERIALIZER_H
#define LIBRETUNER_BYTEORDER_H


#include <type_traits>
#include <utility>
#include <vector>

#include "../support/types.h"

namespace lt {

// Arithmetic type serialization
template<typename T, typename S, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
void serialize(S &s, T t) {
    T swapped = endian::convert<T, endian::current, S::Endian>(t);
    s.write(reinterpret_cast<const uint8_t*>(std::addressof(swapped)), sizeof(T));
}

template<typename T, typename S, typename Allocator>
void serialize(S &s, const std::vector<T, Allocator> &vec) {
    s.serialize(vec.data(), vec.size());
}

template<typename Sink, Endianness endianness = endian::current>
class Serializer {
public:
    static constexpr Endianness Endian = endianness;

    template<typename ...Args>
    Serializer(Args &&...args) : sink(std::forward<Args>(args)...) {

    }

    template<typename T>
    inline void serialize(const T &t) {
        ::lt::serialize(*this, t);
    }

    // Generic array serialization
    template<typename T>
    void serialize(const T *array, std::size_t length) {
        serialize(static_cast<uint32_t>(length));
        std::for_each(array, &array[length], [this](const T &t) {
            serialize(t);
        });
    }

    // Serialize raw bytes
    inline void write(const uint8_t *d, std::size_t length) {
        sink.write(d, length);
    }

private:
    Sink sink;
};

template<typename Source, Endianness endianness = Endianness::Little>
class Deserializer {
public:


};


}


#endif //LIBRETUNER_SERIALIZER_H
