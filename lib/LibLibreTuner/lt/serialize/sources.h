#ifndef LT_SOURCES_H
#define LT_SOURCES_H

#include <vector>
#include <cstdint>
#include <stdexcept>

namespace lt {

    template<class Vector>
    class VectorSource {
    public:
        VectorSource(const Vector &vector) : vector_(vector), it_(vector_.begin()) {}

        void read(uint8_t *d, int length) {
            if (std::distance(it_, vector_.end()) < length) {
                throw std::runtime_error("Cannot deserialize " + std::to_string(length) + " bytes from vector; reaches EOF");
            }
            std::copy(it_, it_ + length, d);
            std::advance(it_, length);
        }
    private:
        const Vector &vector_;
        typename Vector::const_iterator it_;
    };

}

#endif //LT_SOURCES_H
