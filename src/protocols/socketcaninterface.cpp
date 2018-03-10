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

SocketCanInterface::SocketCanInterface(CanInterface::Callbacks& callbacks) : CanInterface(callbacks)
{
    socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
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
        return false;
    }
    
    // TODO: remove EFF/RTR/ERR flags
    message.setMessage(frame.can_id, frame.data, frame.can_dlc);
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
    assert(socket_ != 0);
    
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



void SocketCanInterface::runAsync()
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
        
        callbacks_.onError(ERR_READ, errno);
        return;
    }
    
    callbacks_.onRecv(message);
}



void SocketCanInterface::setNonblocking()
{
    assert(socket_ != 0);
    int flags = fcntl(socket_, F_GETFL, 0);
    fcntl(socket_, F_SETFL, flags | O_NONBLOCK);
}

