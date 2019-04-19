#ifndef LIBRETUNER_SERIALIZER_H
#define LIBRETUNER_BYTEORDER_H


#include <type_traits>
#include <utility>

#include "../support/types.h"

namespace lt {



template<typename Sink, Endianness endianness = endian::current>
class Serializer {
public:
    using Endian = Endianness;

    template<typename ...Args>
    Serializer(Args &&...args) : sink(std::forward<Args>(args)...) {

    }

    template<typename T>
    inline void serialize(const T &t) {
        serialize<T>(*this, t);
    }

    // Serialize raw bytes
    inline void serialize(const uint8_t *d, std::size_t length) {
        sink.write(d, length);
    }

private:
    Sink sink;
};

// Arithmetic type serialization
template<typename T, typename S, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
void serialize(S &s, T t) {
    T swapped = endian::convert<T, endian::current, S::Endian>(t);
    s.serialize(reinterpret_cast<const uint8_t*>(std::addressof(swapped)), sizeof(T));
}

template<typename Source, Endianness endianness = Endianness::Little>
class Deserializer {
public:


};


}


#endif //LIBRETUNER_SERIALIZER_H
