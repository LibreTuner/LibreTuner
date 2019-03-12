#ifndef LT_TABLEEXT_H
#define LT_TABLEEXT_H

#include "support/types.h"
#include "support/util.hpp"
#include "table.h"

namespace lt {
// Serializes table to bytes
class TableSerializer {
public:
	TableSerializer(Endianness endianness) : endianness_(endianness) {}

    template <typename T>
    void serialize(const Table &table, uint8_t *data, std::size_t size,
                   double scale = 1.0) {
        auto serialized = table.serialize<T>(scale);
        if (size < serialized.size() * sizeof(T)) {
            throw std::runtime_error("serialize buffer too small");
        }

		const uint8_t *end = data + size;

        if (endianness == Endianness::Big) {
            for (const T &val : serialized) {
				writeBE<T>(val, data, end);
				data += sizeof(T);
            }
        } else if (endianness == Endianness::Little) {
			for (const T& val : serialized) {
				writeLE<T>(val, data, end);
				data += sizeof(T);
			}
        }
    }

private:
    Endianness endianness_;
};

class TableDeserializer {
public:
	TableDeserializer(Endianness endianness) : endianness_(endianness) {}

	template <typename T>
	void deserialize(Table& table, std::size_t width, std::size_t height, const uint8_t *data, std::size_t size, double scale = 1.0) {
		if (width * height * sizeof(T) < size) {
			throw std::runtime_error("insufficient deserialize buffer size");
		}
		table.initialize<T>(width, height);

		std::vector<T> deserialized(width * height);

		const uint8_t* end = data + size;

		if (endianness == Endianness::Big) {
			for (T& val : deserialized) {
				val = readBE<T>(data, end);
				data += sizeof(T);
			}
		}
		else if (endianness == Endianness::Little) {
			for (T& val : deserialized) {
				val = readLE<T>(data, end);
				data += sizeof(T);
			}
		}
	}

private:
	Endianness endianness_;
};
} // namespace lt

#endif