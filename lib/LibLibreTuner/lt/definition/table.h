#ifndef LT_TABLEDEF_H
#define LT_TABLEDEF_H

#include "../support/types.h"
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace lt
{

enum class AxisType
{
    Linear,
    Memory,
};

struct LinearAxisDefinition
{
    double start;
    double increment;
    int size;
};

struct MemoryAxisDefinition
{
    int size;
};

struct AxisDefinition
{
    std::string name;
    std::string id;
    DataType dataType;

    std::variant<LinearAxisDefinition, MemoryAxisDefinition> def;
};

/* Describes a calibration table. Designed with
 * an inheritance-based definition system in mind.
 * e.g. Mazdaspeed3/6/CX-7 would inherit from a base
 * MZR-DISI definition. This would symplify the current system by
 * eliminating the distinction in models and platforms. */
struct TableDefinition
{
    std::string id;
    std::string name;
    std::string description;
    std::string category;
    std::string unit;
    DataType dataType;
    DataType storedDataType;
    int width{1};
    int height{1};
    double maximum{std::numeric_limits<double>::max()};
    double minimum{std::numeric_limits<double>::lowest()};
    double scale = 1.0;
    std::string axisX;
    std::string axisY;

    // Offsets only show up in models
    std::optional<int> offset;

    inline int byteSize() const noexcept
    {
        return static_cast<int>(dataTypeSize(storedDataType)) * width * height;
    }
};

} // namespace lt

#endif
