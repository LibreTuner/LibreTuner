#ifndef LT_TABLEDEF_H
#define LT_TABLEDEF_H

#include <string>
#include <variant>
#include "../support/types.h"

namespace lt {

enum class AxisType {
	Linear,
	Memory,
};

struct LinearAxisDefinition {
    double start;
    double increment;
};

struct MemoryAxisDefinition {
    int size;
};

struct AxisDefinition {
	std::string name;
	std::string id;
	DataType dataType;

	std::variant<LinearAxisDefinition, MemoryAxisDefinition> def;
};

struct TableDefinition {
	int id{-1};
	std::string name;
	std::string description;
	std::string category;
	DataType dataType;
	DataType storedDataType;
	int width{1};
	int height{1};
	double maximum;
	double minimum;
	double scale = 1.0;
	std::string axisX;
	std::string axisY;

	inline int byteSize() const noexcept {
		return dataTypeSize(storedDataType) * width * height;
	}
};

} // namespace lt

#endif
