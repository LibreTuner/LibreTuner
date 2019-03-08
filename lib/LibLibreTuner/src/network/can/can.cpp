#include "can.h"

#include <cassert>

namespace lt {
namespace network {

void Can::send(uint32_t id, const uint8_t* data, size_t length)
{
    assert(data != nullptr);
    
    return send(CanMessage(id, data, length));
}



CanMessage::CanMessage(uint32_t id, const uint8_t* message, std::size_t length)
{
    setMessage(id, message, length);
}



void CanMessage::setMessage(const uint8_t* message, std::size_t length)
{
    assert(length <= 8);
    std::copy(message, message + length, message_.begin());
    length_ = length;
}


} // namespace network
} // namespace lt
