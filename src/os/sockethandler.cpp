#include "sockethandler.h"
#include <cassert>
#include <iostream>

SocketHandler::SocketHandler()
{

}



SocketHandler::~SocketHandler()
{
    if (thread_.joinable())
    {
        running_ = false;
        thread_.join();
    }
}



SocketHandler * SocketHandler::get()
{
    static SocketHandler gSocketHandler_;
    return &gSocketHandler_;
}



void SocketHandler::addSocket(Socket* socket)
{
    sockets_.push_back(socket);
}



void SocketHandler::removeSocket(Socket* socket)
{
    for (auto it = sockets_.begin(); it != sockets_.end(); ++it)
    {
        if (*it == socket)
        {
            sockets_.erase(it);
            return;
        }
    }
}



void SocketHandler::initialize()
{
    assert(!thread_.joinable());
    running_ = true;
    thread_ = std::thread(&SocketHandler::run, this);
}



void SocketHandler::run()
{
    fd_set rdfds;
    timeval timeout;
    while (running_)
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;
        FD_ZERO(&rdfds);
        
        int nfds = 0;
        
        for (Socket *socket : sockets_)
        {
            FD_SET(socket->fd(), &rdfds);
            if (socket->fd() > nfds)
            {
                nfds = socket->fd();
            }
        }
        
        int res = select(nfds + 1, &rdfds, nullptr, nullptr, &timeout);
        if (res == -1)
        {
            break;
        }
        
        if (res == 0)
        {
            // The timeout expired. The sets will be empty
            continue;
        }
        
        for (Socket *socket : sockets_)
        {
            if (FD_ISSET(socket->fd(), &rdfds))
            {
                socket->onRead();
            }
        }
    }
}
