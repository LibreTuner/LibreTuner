#ifndef LT_TABLEEXT_H
#define LT_TABLEEXT_H

#include "../support/types.h"
#include "../support/util.hpp"
#include "table.h"
#include "../definition/table.h"

namespace lt {
// Serializes table to bytes
/*
class TableSerializer {
public:
    TableSerializer(Endianness endianness) : endianness_(endianness) {}

    template <typename T>
    std::vector<uint8_t> serialize(const Table &table, double scale = 1.0) {
        std::vector<T> serialized = table.serialize<T>(scale);

		std::vector<uint8_t> bytes(serialized.size() * sizeof(T));

        if (endianness_ == Endianness::Big) {
			writeBE<T>(serialized.begin(), serialized.end(), bytes.begin());
        } else if (endianness_ == Endianness::Little) {
			writeLE<T>(serialized.begin(), serialized.end(), bytes.begin());
        }
    }

private:
    Endianness endianness_;
};

struct SerializedTable {
	std::size_t id;
	std::vector<uint8_t> data;
};

class TableDeserializer {
public:
    TableDeserializer(Endianness endianness) : endianness_(endianness) {}

    template <typename T>
    void deserialize(Table &table, std::size_t width, std::size_t height,
                     const uint8_t *data, std::size_t size,
                     double scale = 1.0) {
        if (width * height * sizeof(T) > size) {
            throw std::runtime_error("insufficient deserialize buffer size");
        }
        table.initialize<T>(width, height);

        std::vector<T> deserialized(width * height);

        const uint8_t *end = data + size;

        if (endianness_ == Endianness::Big) {
            for (T &val : deserialized) {
                val = readBE<T>(data, end);
                data += sizeof(T);
            }
        } else if (endianness_ == Endianness::Little) {
            for (T &val : deserialized) {
                val = readLE<T>(data, end);
                data += sizeof(T);
            }
        }

        table.deserialize(deserialized.begin(), deserialized.end(), scale);
    }

private:
    Endianness endianness_;
};
*/

struct TableInitializer {
	Table &table;
	std::size_t width, height;

	template<typename T>
	void operator()() {
		table.initialize<T>(width, height);
	}
};

inline void initializeTable(Table &table, const TableDefinition &def) {
	table.setScale(def.scale);
	table.setName(def.name);
	table.setDescription(def.description);
	table.setBounds(TableBounds<double>{def.minimum, def.maximum});

	TableInitializer initializer{table, def.width, def.height};
	datatypeToType(def.storedDataType, initializer);
}

} // namespace lt

#endif
