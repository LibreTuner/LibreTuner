#ifndef SOCKETCAN_H
#define SOCKETCAN_H

#include "can.h"
#include "os/socket.h"

#include <string>

namespace lt {
namespace network {

class SocketCan : public Can {
public:
    SocketCan(SocketCan &) = delete;
    SocketCan(const SocketCan &) = delete;
    SocketCan(SocketCan &&) = delete;
    
    ~SocketCan() override = default;
    
    SocketCan(const std::string &ifname);

    // Can interface
public:
    virtual void send(const CanMessage &message) override;

    /* Returns false if the timeout expired and no message was read. */
    virtual bool recv(CanMessage &message,
                      std::chrono::milliseconds timeout) override;

private:
    os::Socket socket_;
};

} // namespace network
} // namespace lt

#endif // SOCKETCAN_H
