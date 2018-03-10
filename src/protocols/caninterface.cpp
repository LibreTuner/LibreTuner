#include "caninterface.h"
#include <cassert>
#include <cstring>
#include <sstream>


void CanMessage::setMessage(uint32_t id, const uint8_t* message, uint8_t length)
{
    assert(length <= 8);
    id_ = id;
    messageLength_ = length;
    memcpy(message_, message, length);
}



std::string CanMessage::strMessage() const
{
    std::stringstream ss;
    for (uint8_t c = 0; c < length(); ++c)
    {
        if (c != 0)
        {
            ss << "  ";
        }
        ss << std::hex << static_cast<unsigned int>(message_[c]);
    }
    
    return ss.str();
}



CanMessage::CanMessage()
{
    memset(message_, 0, 8);
}



CanMessage::CanMessage(uint32_t id, const uint8_t* message, uint8_t length)
{
    setMessage(id, message, length);
}



CanInterface::CanInterface(Callbacks *callbacks) : callbacks_(callbacks)
{
}



bool CanInterface::send(int id, const uint8_t* message, uint8_t len)
{
    return send(CanMessage(id, message, len));
}
