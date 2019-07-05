#ifndef LIBRETUNER_BYTEORDER_H
#define LIBRETUNER_BYTEORDER_H

#include <type_traits>
#include <utility>

#include "../support/types.h"

namespace lt
{

template <Endianness e> class ByteOrder
{
};

template <> class ByteOrder<Endianness::Little>
{
    template <typename T> T convert(T t) {}
};

} // namespace lt

#endif // LIBRETUNER_BYTEORDER_H
