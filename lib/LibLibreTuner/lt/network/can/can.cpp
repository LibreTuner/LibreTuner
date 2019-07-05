#include "can.h"

#include <cassert>

namespace lt
{
namespace network
{

void Can::send(uint32_t id, const uint8_t * data, size_t length)
{
    assert(data != nullptr);
    assert(length <= 8);

    return send(CanMessage(id, data, static_cast<uint8_t>(length)));
}

CanMessage::CanMessage(uint32_t id, const uint8_t * message, uint8_t length)
{
    setMessage(id, message, length);
}

void CanMessage::setMessage(const uint8_t * message, uint8_t length)
{
    assert(length <= 8);
    std::copy(message, message + length, message_.begin());
    length_ = length;
}

void CanMessage::pad() noexcept
{
    for (auto it = message_.begin() + length_; it < message_.end(); ++it)
    {
        *it = 0;
    }
    length_ = 8;
}

} // namespace network
} // namespace lt
