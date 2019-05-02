#ifndef SOCKET_H
#define SOCKET_H

#ifdef WITH_SOCKETCAN

#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <cstdint>
#include <cassert>
#include <utility>
#include <stdexcept>

namespace lt {
namespace os {
    
using Socket_t = int;
using Address_t = sockaddr;
using SocketLen_t = socklen_t;

// POSIX-compliant socket
class Socket
{
public:
    Socket() = default;
    // Same as create(domain, type, protocol)
    Socket(int domain, int type, int protocol) { create(domain, type, protocol); }
    // Takes ownership of a socket
    explicit Socket(Socket_t socket) : socket_{socket}, valid_{true} {}
    
    Socket(const Socket&) = delete;
    Socket &operator=(const Socket&) = delete;
    
    // Creates the socket. Throws an exception if the socket cannot be created.
    void create(int domain, int type, int protocol);
    
    // Returns true if the socket is valid
    inline bool valid() const { return valid_; }
    
    // Returns the internal file descriptor
    Socket_t descriptor() const { return socket_; }
    
    // Binds an address to the socket. Throws an exception on failure
    void bind(const Address_t *address, SocketLen_t address_len);
    
    std::size_t recv(void *buffer, int length, int flags);
    ssize_t recvNoExcept(void *buffer, int length, int flags) noexcept;
    
    // Throws an exception on failure or if less than `length` bytes are sent
    void send(void *buffer, int length, int flags);
    ssize_t sendNoExcept(void *buffer, int length, int flags) noexcept;
    
    void setsockopt(int level, int option_name, const void *option_value, SocketLen_t option_len);
    
    template<typename ...Args>
    int ioctl(unsigned long request, Args ...args) {
        assert(valid());
        int ret = ::ioctl(socket_, request, std::forward<Args>(args)...);
        if (ret == -1) {
            throw std::runtime_error(strerror(errno));
        }
        return ret;
    }
    
    void close();
    
private:
    Socket_t socket_{0};
    bool valid_{false};
};

} // namespace os
} // namespace lt

#endif

#endif // SOCKET_H
