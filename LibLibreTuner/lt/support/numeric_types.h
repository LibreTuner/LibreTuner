#ifndef LT_NUMERIC_TYPES_H
#define LT_NUMERIC_TYPES_H

// Based off of
// https://github.com/REDasmOrg/REDasm-Library/blob/master/redasm/types/numeric_type.h

#include "endianness.h"

namespace lt
{

template <typename T, Endianness endianness> class numeric_type
{
    static_assert(std::is_arithmetic_v<T>, "Type must be arithmetic");

public:
private:
};

} // namespace lt

#endif // LT_NUMERIC_TYPES_H
