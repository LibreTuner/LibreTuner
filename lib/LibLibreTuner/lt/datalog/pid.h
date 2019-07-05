#ifndef DATALOG_PID_H
#define DATALOG_PID_H

#include <cstdint>
#include <string>

namespace lt
{
struct Pid
{
    uint16_t code;
    std::string name;
    std::string description;
    std::string formula;
    std::string unit;
};
} // namespace lt

#endif // DATALOG_PID_H
