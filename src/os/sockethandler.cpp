#include "sockethandler.h"
#include <cassert>

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
    std::unique_lock<std::mutex> lk(cv_m_);
    sockets_.push_back(socket);
    runLooped_ = false;
    cv_.wait(lk, [this]{return runLooped_ == true; });
    
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
        
        {
            // trigger runLooped_ for the addSocket() block
            std::lock_guard<std::mutex> lk(cv_m_);
            runLooped_ = true;
        }
        cv_.notify_all();
        
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
