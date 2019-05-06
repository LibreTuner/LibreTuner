#ifndef LT_SINKS_H
#define LT_SINKS_H

#include <vector>

namespace lt {

template<class Vector>
class VectorSink {
public:
    VectorSink(Vector &vector) : vector_(vector) {}

    void write(const uint8_t *d, int length) {
        vector_.insert(vector_.end(), d, d + length);
    }
private:
    Vector &vector_;
};

}

#endif //LT_SINKS_H
