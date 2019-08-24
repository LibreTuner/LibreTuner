#ifndef LIBRETUNER_UNIT_H
#define LIBRETUNER_UNIT_H

#include <string>
#include <utility>
#include <vector>

namespace lt::unit
{

enum class Unit
{
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
    constexpr static UnitName units[] = {
        {"kpa", Unit::Pressure_Kpa},
        {"bar", Unit::Pressure_Bar},
        {"psi", Unit::Pressure_Psi}
    };

    constexpr static auto name = "pressure";

    /* Converts value into target unit. */
    double convert(double val);

private:
    Unit base_;
    Unit target_;
};

}
#endif // LIBRETUNER_UNIT_H
