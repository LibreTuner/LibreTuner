#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <vector>
#include <thread>

class Socket {
public:
    /* Returns the file descriptor for the socket */
    virtual int fd() =0;
    
    /* Called when the socket can be read */
    virtual void onRead() =0;
};


/**
 * Handles asynchronous sockets on *nix hosts
 */
class SocketHandler
{
public:
    /* Returns the global SocketHandler object */
    static SocketHandler *get();
    
    void addSocket(Socket *socket);
    void removeSocket(Socket *socket);
    
    /* Starts the thread for handling socket reads/writes */
    void initialize();
    
    ~SocketHandler();
    
private:
    std::thread thread_;
    std::vector<Socket*> sockets_;

    
    void run();
    SocketHandler();
    
    bool running_;
};

#endif // SOCKETHANDLER_H