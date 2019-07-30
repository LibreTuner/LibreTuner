#ifndef LIBRETUNER_VIEW_H
#define LIBRETUNER_VIEW_H

#include "../support/endianness.h"
#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>

#include "memorybuffer.h"

namespace lt
{
class View
{
public:
    View(MemoryBuffer & buffer, int offset, int size);

    template <typename T, Endianness endianness> T get(int offset = 0) const
    {
        if (offset + sizeof(T) > size())
            throw std::runtime_error("TuneView::get(): index out of range");
        auto it = std::next(buffer_.begin(), offset_ + offset);

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

    template <typename T, Endianness endianness> void set(T t, int offset = 0)
    {
        if (offset + sizeof(T) > size())
            throw std::runtime_error("TuneView::get(): index out of range");
        auto it = std::next(buffer_.begin(), offset_ + offset);

        T val = endian::convert<T, endian::current, endianness>(t);
        uint8_t * repr = reinterpret_cast<uint8_t *>(&val);
        for (std::size_t i = 0; i < sizeof(T); ++i)
        {
            *it = repr[i];
            std::advance(it, 1);
        }
    }

    inline int size() const { return size_; }
    inline uint8_t * operator*() noexcept { return buffer_.data(); }
    inline uint8_t & operator[](int index) { return buffer_[index]; }
    inline const uint8_t & operator[](int index) const
    {
        return buffer_[index];
    }
    inline uint8_t * data() noexcept { return buffer_.data(); }
    inline const uint8_t * data() const noexcept { return buffer_.data(); }

    MemoryBuffer::iterator begin()
    {
        return std::next(buffer_.begin(), offset_);
    }
    MemoryBuffer::iterator end()
    {
        return std::next(buffer_.begin(), offset_ + size_);
    }
    MemoryBuffer::const_iterator cbegin() const
    {
        return std::next(buffer_.cbegin(), offset_);
    }
    MemoryBuffer::const_iterator cend() const
    {
        return std::next(buffer_.cbegin(), offset_ + size_);
    }

    View view(int offset, int size);

private:
    MemoryBuffer & buffer_;
    int offset_, size_;
};

} // namespace lt

#endif // LIBRETUNER_VIEW_H
