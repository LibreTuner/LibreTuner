#ifndef LIBRETUNER_VIEW_H
#define LIBRETUNER_VIEW_H

#include <cassert>
#include <memory>
#include <stdexcept>
#include <support/endianness.h>

#include "rom.h"

namespace lt
{

// Tune view
class View
{
public:
    View(TunePtr tune, int offset, int size)
    {
        assert(offset >= 0);
        assert(size >= 0);

        if (offset + size >= tune->size())
            throw std::runtime_error("TuneView range exceeds tune size");

        pointer_ = std::next(tune->begin(), offset);
        end_ = std::next(pointer_, size);
    }

    inline int size() const { return std::distance(pointer_, end_); }

    template <typename T, Endianness endianness> T get(int offset) const
    {
        if (offset > size())
            throw std::runtime_error("TuneView::get(): index out of range");
        auto it = std::next(pointer_, offset);

        T val;
        uint8_t * repr = reinterpret_cast<uint8_t *>(&val);

        // Unroll this loop, compiler!
        for (std::size_t i = 0; i < sizeof(T); ++i)
        {
            repr[i] = *it;
            std::advance(it, 1);
        }
        return endian::convert<T, endianness, endian::current>(val);
    }

    template <typename T, Endianness endianness> void set(int offset, T t)
    {
        if (offset > size())
            throw std::runtime_error("TuneView::get(): index out of range");
        auto it = std::next(pointer_, offset);

        T val = endian::convert<T, endian::current, endianness>(t);
        uint8_t * repr = reinterpret_cast<uint8_t *>(&val);
        for (std::size_t i = 0; i < sizeof(T); ++i)
        {
            *it = repr[i];
            std::advance(it, 1);
        }
    }

private:
    Tune::iterator pointer_;
    Tune::iterator end_;
};

} // namespace lt

#endif // LIBRETUNER_VIEW_H
