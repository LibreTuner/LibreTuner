#ifndef LT_TYPES_H
#define LT_TYPES_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "endianness.h"
#include "util.hpp"

// Ugly macro
#define ENABLE_BITMASK(Enum)                                                   \
    inline Enum operator|(Enum lhs, Enum rhs)                                  \
    {                                                                          \
        using UType = std::underlying_type_t<Enum>;                            \
        return static_cast<Enum>(static_cast<UType>(lhs) |                     \
                                 static_cast<UType>(rhs));                     \
    }                                                                          \
    inline Enum operator&(Enum lhs, Enum rhs)                                  \
    {                                                                          \
        using UType = std::underlying_type_t<Enum>;                            \
        return static_cast<Enum>(static_cast<UType>(lhs) &                     \
                                 static_cast<UType>(rhs));                     \
    }                                                                          \
    inline Enum & operator|=(Enum & lhs, Enum rhs)                             \
    {                                                                          \
        lhs = lhs | rhs;                                                       \
        return lhs;                                                            \
    }                                                                          \
    inline Enum & operator&=(Enum & lhs, Enum rhs)                             \
    {                                                                          \
        lhs = lhs & rhs;                                                       \
        return lhs;                                                            \
    }

namespace lt
{
// Datatype
enum class DataType
{
    Invalid,
    Uint8,
    Uint16,
    Uint32,
    Float,
    Int8,
    Int16,
    Int32,
};

template<DataType T>
struct DataTypeTraits {};

template<>
struct DataTypeTraits<DataType::Uint8>
{
    using Type = uint8_t;
};

template<>
struct DataTypeTraits<DataType::Uint16>
{
    using Type = uint16_t;
};

template<>
struct DataTypeTraits<DataType::Uint32>
{
    using Type = uint32_t;
};

template<>
struct DataTypeTraits<DataType::Int8>
{
    using Type = int8_t;
};

template<>
struct DataTypeTraits<DataType::Int16>
{
    using Type = int16_t;
};

template<>
struct DataTypeTraits<DataType::Int32>
{
    using Type = int32_t;
};

template<>
struct DataTypeTraits<DataType::Float>
{
    using Type = float;
};

inline DataType datatype_from_string(const std::string & type)
{
    if (type == "float")
        return lt::DataType::Float;
    if (type == "uint8")
        return lt::DataType::Uint8;
    if (type == "uint16")
        return lt::DataType::Uint16;
    if (type == "uint32")
        return lt::DataType::Uint32;
    if (type == "int8")
        return lt::DataType::Int8;
    if (type == "int16")
        return lt::DataType::Int16;
    if (type == "int32")
        return lt::DataType::Int32;
    return lt::DataType::Invalid;
}

template <typename Func, typename... Args>
inline void datatypeToType(DataType type, Func && func, Args &&... args)
{
    switch (type)
    {
    case DataType::Float:
        func.template operator()<float>(std::forward<Args>(args)...);
        break;
    case DataType::Uint8:
        func.template operator()<uint8_t>(std::forward<Args>(args)...);
        break;
    case DataType::Uint16:
        func.template operator()<uint16_t>(std::forward<Args>(args)...);
        break;
    case DataType::Uint32:
        func.template operator()<uint32_t>(std::forward<Args>(args)...);
        break;
    case DataType::Int8:
        func.template operator()<int8_t>(std::forward<Args>(args)...);
        break;
    case DataType::Int16:
        func.template operator()<int16_t>(std::forward<Args>(args)...);
        break;
    case DataType::Int32:
        func.template operator()<int32_t>(std::forward<Args>(args)...);
        break;
    case DataType::Invalid:
        break;
    }
}

template<DataType Selected, DataType ...Args>
struct DataTypeExecutor_
{
    static int size(DataType dt)
    {
        if (dt == Selected)
            return sizeof(typename DataTypeTraits<Selected>::Type);
        if constexpr (sizeof...(Args) == 0U)
            return 0;
        else
            return DataTypeExecutor_<Args...>::size(dt);
    }
};

using DataTypeExecutor = DataTypeExecutor_<DataType::Float, DataType::Uint8, DataType::Uint16, DataType::Uint32, DataType::Int8, DataType::Int16, DataType::Int32>;

inline std::size_t dataTypeSize(DataType type)
{
    return DataTypeExecutor::size(type);
}

enum class DataLinkType
{
    SocketCan,
    PassThru,
    Elm,
    Invalid,
};

enum class NetworkProtocol : unsigned
{
    None = 0,
    Can = 0x1,
    IsoTp = 0x2,
};
ENABLE_BITMASK(NetworkProtocol)

} // namespace lt

#endif
