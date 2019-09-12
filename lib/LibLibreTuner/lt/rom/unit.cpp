#include "unit.h"

#include <stdexcept>
#include <support/util.hpp>

namespace lt
{

template <typename G> class GroupImpl : public UnitGroup
{
public:
    explicit GroupImpl(const std::string & base)
    {
        for (const auto & unit : G::units)
        {
            if (unit.name == base)
            {
                base_ = unit.unit;
                target_ = unit.unit;
                return;
            }
        }
        throw std::runtime_error("unknown unit '" + base + "'");
    }

    explicit GroupImpl(Unit base) : base_(base), target_(base) {}

    double convert(double value) override { return G::convert(base_, target_, value); }
    std::vector<UnitName> units() override
    {
        // TODO: Don't copy the table each time.
        return std::vector<UnitName>(G::units.begin(), G::units.end());
    }

private:
    Unit base_;
    Unit target_;
};

std::unique_ptr<UnitGroup> make_group(const std::string & name)
{
    auto sep = std::find(name.begin(), name.end(), '.');
    if (sep == name.end())
        return std::unique_ptr<UnitGroup>();

    std::string unit(std::next(sep), name.end());
    std::string base(name.begin(), sep);
    lowercase_string(base);
    lowercase_string(unit);

    if (base == Pressure::name)
        return std::make_unique<GroupImpl<Pressure>>(unit);

    return std::unique_ptr<UnitGroup>();
}

}