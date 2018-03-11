#include "socketcaninterface.h"

#include <cassert>
#include <cstring>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>
#include <libretune.h>

SocketCanInterface::SocketCanInterface(CanInterface::Callbacks *callbacks) : CanInterface(callbacks)
{
    
}



SocketCanInterface::SocketCanInterface(CanInterface::Callbacks* callbacks, const char* ifname) : CanInterface(callbacks)
{
    bind(ifname);
}



SocketCanInterface::~SocketCanInterface()
{
    if (socket_ != 0)
    {
        close();
    }
}



bool SocketCanInterface::recv(CanMessage &message)
{
    assert(socket_ != 0);
    
    can_frame frame;
    
    int nbytes = read(socket_, &frame, sizeof(can_frame));
    if (nbytes < 0)
    {
        lastError_ = ERR_READ;
        lastErrno_ = errno;
        return false;
    }
    
    // TODO: remove EFF/RTR/ERR flags
    message.setMessage(frame.can_id, frame.data, frame.can_dlc);
    
    // Add message to log
    LibreTune::get()->canLog()->addMessage(message);
    
    return true;
}



bool SocketCanInterface::send(const CanMessage &message)
{
    assert(socket_ != 0);
    
    can_frame frame;
    
    memset(&frame, 0, sizeof(can_frame));
    frame.can_dlc = message.length();
    frame.can_id = message.id();
    memcpy(frame.data, message.message(), message.length());
    
    return (write(socket_, &frame, sizeof(can_frame)) == sizeof(can_frame));
}



bool SocketCanInterface::bind(const char* ifname)
{
    socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_ == -1)
    {
        lastError_ = ERR_SOCKET;
        lastErrno_ = errno;
        socket_ = 0;
        return false;
    }
    
    sockaddr_can addr;
    ifreq ifr;
    
    strcpy(ifr.ifr_name, ifname);
    if (ioctl(socket_, SIOCGIFINDEX, &ifr) != 0)
    {
        return false;
    }
    
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    
    if (::bind(socket_, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        lastError_ = ERR_SOCKET;
        lastErrno_ = errno;
        return false;
    }
    
    return true;
}



void SocketCanInterface::close()
{
    assert(socket_ != 0);
    SocketHandler::get()->removeSocket(this);
    ::close(socket_);
    socket_ = 0;
}



void SocketCanInterface::start()
{
    assert(socket_ != 0);
    
    setNonblocking();
    SocketHandler::get()->addSocket(this);
}



int SocketCanInterface::fd()
{
    return socket_;
}



void SocketCanInterface::onRead()
{
    CanMessage message;
    if (!recv(message))
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }
        
        callOnError(ERR_READ, errno);
        return;
    }
    
    callOnRecv(message);
}



void SocketCanInterface::setNonblocking()
{
    assert(socket_ != 0);
    int flags = fcntl(socket_, F_GETFL, 0);
    fcntl(socket_, F_SETFL, flags | O_NONBLOCK);
}

