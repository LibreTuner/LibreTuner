#ifndef LT_AUTH
#define LT_AUTH

#include <string>

namespace lt
{
namespace auth
{

struct Options
{
    std::string key;
    uint8_t session = 0x87;
};

} // namespace auth
} // namespace lt

#endif