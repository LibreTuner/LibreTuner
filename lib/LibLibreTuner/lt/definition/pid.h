#ifndef LT_PIDDEF_H
#define LT_PIDDEF_H

#include <cstdint>
#include <string>

namespace lt
{

struct Pid
{
    std::string name;
    std::string description;
    std::string formula;
    std::string unit;
    uint32_t id;
    uint16_t code;

    bool valid = false;
};

} // namespace lt

#endif
