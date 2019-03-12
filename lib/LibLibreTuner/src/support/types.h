#ifndef LT_TYPES_H
#define LT_TYPES_H

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace lt {

enum class Endianness { Big, Little };

enum class DataType {
	Uint8,
	Uint16,
	Uint32,
	Float,
	Int8,
	Int16,
	Int32,
};


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

} // namespace lt

#endif