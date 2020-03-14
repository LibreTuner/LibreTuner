#ifndef LIBRETUNER_UNIT_H
#define LIBRETUNER_UNIT_H

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <array>

namespace lt
{

enum class Unit
{
    None,
    Pressure_Kpa,
    Pressure_Bar,
    Pressure_Psi,
};

struct UnitName
{
    const char * name;
    Unit unit;
};

class Pressure
{
public:
    constexpr static std::array<UnitName, 3> units = {{
        {"kpa", Unit::Pressure_Kpa},
        {"bar", Unit::Pressure_Bar},
        {"psi", Unit::Pressure_Psi},
    }};

    constexpr static auto name = "pressure";

    /* Converts value into target unit. */
    static double convert(Unit base, Unit target, double val)
    {
        if (base == target)
            return val;

        // Convert val to kPa
        switch (base)
        {
        case Unit::Pressure_Bar:
            val *= 100.0;
            break;
        case Unit::Pressure_Psi:
            val *= 6.8947572932;
            break;
        default:
            break;
        }

        // Convert val to target
        switch (target)
        {
        case Unit::Pressure_Bar:
            val /= 100.0;
            break;
        case Unit::Pressure_Psi:
            val /= 6.8947572932;
            break;
        default:
            break;
        }
        return val;
    }
};

class UnitGroup
{
public:
    virtual ~UnitGroup() = default;

    virtual double convert(double value) =0;

    /* Returns a copy of units names. */
    virtual const std::vector<UnitName> &units() =0;
};

/* Makes a unit group using base unit. */
std::unique_ptr<UnitGroup> make_group(const std::string & name);

}
#endif // LIBRETUNER_UNIT_H
