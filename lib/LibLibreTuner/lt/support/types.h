#ifndef LT_TYPES_H
#define LT_TYPES_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "endianness.h"

namespace lt {

enum class DataType {
    Uint8,
    Uint16,
    Uint32,
    Float,
    Int8,
    Int16,
    Int32,
};

template <typename Func>
inline void datatypeToType(DataType type, Func &func) {
    switch (type) {
    case DataType::Float:
        func.template operator()<float>();
        break;
    case DataType::Uint8:
        func.template operator()<uint8_t>();
        break;
    case DataType::Uint16:
        func.template operator()<uint16_t>();
        break;
    case DataType::Uint32:
        func.template operator()<uint32_t>();
        break;
    case DataType::Int8:
        func.template operator()<int8_t>();
        break;
    case DataType::Int16:
        func.template operator()<int16_t>();
        break;
    case DataType::Int32:
        func.template operator()<int32_t>();
        break;
    }
}

inline std::size_t dataTypeSize(DataType type) {
    switch (type) {
    case DataType::Float:
        return sizeof(float);
    case DataType::Uint8:
        return sizeof(uint8_t);
    case DataType::Uint16:
        return sizeof(uint16_t);
    case DataType::Uint32:
        return sizeof(uint32_t);
    case DataType::Int8:
        return sizeof(int8_t);
    case DataType::Int16:
        return sizeof(int16_t);
    case DataType::Int32:
        return sizeof(int32_t);
    }

    assert(false && "unimplemented");
}

enum class DataLinkType {
	SocketCan,
	PassThru,
	Elm,
	Invalid,
};

enum class NetworkProtocol {
	None = 0,
	Can = 0x1,
	IsoTp = 0x2,
};

inline NetworkProtocol operator|(NetworkProtocol lhs, NetworkProtocol rhs) {
	using DType = std::underlying_type<NetworkProtocol>::type;
	return static_cast<NetworkProtocol>(static_cast<DType>(lhs) |
		static_cast<DType>(rhs));
}

inline NetworkProtocol& operator|=(NetworkProtocol& lhs, NetworkProtocol rhs) {
	lhs = lhs | rhs;
	return lhs;
}

inline NetworkProtocol operator&(NetworkProtocol lhs, NetworkProtocol rhs) {
	using DType = std::underlying_type<NetworkProtocol>::type;
	return static_cast<NetworkProtocol>(static_cast<DType>(lhs) &
		static_cast<DType>(rhs));
}

} // namespace lt

#endif
