#include "canhandler.h"
#include "socketcaninterface.h"
#include <memory>
#include <iostream>

CanHandler::CanHandler()
{
}


bool CanHandler::startSocketCan(const char* name)
{
    std::unique_ptr<SocketCanInterface> can(new SocketCanInterface(*this));
    
    if (!can->valid())
    {
        std::cerr << "Could not start SocketCan interface: could not create socket" << std::endl;
        return false;
    }
    
    if (!can->bind(name))
    {
        std::cerr << "Could not start SocketCan interface: could not bind" << std::endl;
        return false;
    }
    
    can->runAsync();
    can_ = std::unique_ptr<CanInterface>(can.release());
    
    return true;
}



void CanHandler::onError(CanInterface::CanError error, int err)
{
    std::cerr <<  "CAN Interface error" << std::endl;
}



void CanHandler::onRecv(const CanMessage& message)
{
    log_.addMessage(message);
}
