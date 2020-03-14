#include "view.h"

#include <cassert>
#include <stdexcept>

namespace lt
{

View::View(MemoryBuffer & buffer, int offset, int size)
    : buffer_(buffer), offset_(offset), size_(size)
{
    assert(offset_ >= 0);
    assert(size_ >= 0);

    if (offset_ + size_ >= buffer_.size())
        throw std::runtime_error("view range exceeds buffer size");
}

View View::view(int offset, int size) {
    return View(buffer_, offset_ + offset, size);
}
}