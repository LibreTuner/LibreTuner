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



CanInterface::CanInterface(Callbacks *callbacks)
{
    if (callbacks != nullptr)
    {
        callbacks_.push_back(callbacks);
    }
}



bool CanInterface::send(int id, const uint8_t* message, uint8_t len)
{
    return send(CanMessage(id, message, len));
}



std::string CanInterface::strError(CanInterface::CanError error, int err)
{
    switch(error)
    {
        case ERR_SUCCESS:
            return "success";
        case ERR_SOCKET:
        {
            std::stringstream ss;
            ss << "failed to create socket: " << strerror(err);
            return ss.str();
        }
        case ERR_READ:
        {
            std::stringstream ss;
            ss << "failed to read: " << strerror(err);
            return ss.str();
        }
        case ERR_WRITE:
            return "failed to transmit message";
        default:
            break;
    }
    
    return "unknown. You should not see this. If you do, please submit a pull request";
}



void CanInterface::callOnRecv(const CanMessage& message)
{
    for (Callbacks *c : callbacks_)
    {
        c->onRecv(message);
    }
}



void CanInterface::callOnError(CanInterface::CanError error, int err)
{
    for (Callbacks *c : callbacks_)
    {
        c->onError(error, err);
    }
}



void CanInterface::removeCallbacks(CanInterface::Callbacks* callbacks)
{
    for (auto it = callbacks_.begin(); it != callbacks_.end(); ++it)
    {
        if (*it == callbacks)
        {
            callbacks_.erase(it);
            return;
        }
    }
}



std::string CanInterface::strError()
{
    return strError(lastError_, lastErrno_);
}


