#include "memorybuffer.h"
#include "view.h"

namespace lt
{
View MemoryBuffer::view() { return View(*this, 0, size()); }

View MemoryBuffer::view(int offset, int size)
{
    return View(*this, offset, size);
}
}