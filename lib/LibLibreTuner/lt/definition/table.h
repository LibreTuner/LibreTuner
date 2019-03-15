#ifndef LT_TABLEDEF_H
#define LT_TABLEDEF_H

#include <string>
#include "../support/types.h"

namespace lt {

enum class AxisType {
	Linear,
	Memory,
};

struct AxisDefinition {
	std::string name;
	std::string id;
	AxisType type;
	DataType dataType;
	union {
		struct {
			double start;
			double increment;
		};
		struct {
			std::size_t size;
		};
	};
};

struct TableDefinition {
	int id{-1};
	std::string name;
	std::string description;
	std::string category;
	DataType dataType;
	DataType storedDataType;
	std::size_t width{1};
	std::size_t height{1};
	double maximum;
	double minimum;
	double scale = 1.0;
	std::string axisX;
	std::string axisY;

	inline std::size_t byteSize() const noexcept {
		return dataTypeSize(storedDataType) * width * height;
	}
};

} // namespace lt

#endif