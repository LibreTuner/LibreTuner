#ifndef SOCKETCANINTERFACE_H
#define SOCKETCANINTERFACE_H

#include <thread>
#include "caninterface.h"
#include "../os/sockethandler.h"

/*
 * SocketCan interface for linux hosts
 */
class SocketCanInterface : public CanInterface, public Socket
{
public:
    SocketCanInterface(CanInterface::Callbacks *callbacks = nullptr);
    /* Creates a socket and attempts to bind to an interface.
     * For errors, check valid() and lastError() */
    SocketCanInterface(CanInterface::Callbacks *callbacks, const std::string &ifname);
    
    ~SocketCanInterface();
    
    bool send(const CanMessage &message) override;
    
    bool recv(CanMessage &message) override;
    
    /* Closes the socket */
    void close();
    
    /* Starts the read loop thread for asynchronous operations */
    void start() override;
    
    /* Binds the socket to a SocketCAN interface. Returns false
     * if an error occured. */
    bool bind(const std::string &ifname);
    
    /* Returns true if the socket is ready for reading/writing */
    bool valid() override
    {
        return socket_ > 0;
    }
    
    /* Sets the socket as nonblocking for async operations */
    void setNonblocking();
    
    /* Socket functions */
    int fd() override;
    void onRead() override;
    
private:
    int socket_ = 0;
};

#endif // SOCKETCANINTERFACE_H
