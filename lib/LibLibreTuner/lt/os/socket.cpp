#include "socket.h"

#ifdef WITH_SOCKETCAN

#include <cassert>
#include <stdexcept>

#include <unistd.h>

namespace lt::os {

static inline void throwErrno() { throw std::runtime_error(strerror(errno)); }

void Socket::create(int domain, int type, int protocol) {
    // Close previously opened socket
    close();

    socket_ = ::socket(domain, type, protocol);
    if (socket_ == -1) {
        valid_ = false;
        throwErrno();
    }
    valid_ = true;
}

void Socket::close() {
    if (valid_) {
        ::close(socket_);
        valid_ = false;
    }
}

void Socket::bind(const Address_t *address, SocketLen_t address_len) {
    assert(valid());
    if (::bind(socket_, address, address_len) == -1) {
        throwErrno();
    }
}

ssize_t Socket::recvNoExcept(void *buffer, int length, int flags) noexcept {
    assert(valid());
    return ::recv(socket_, buffer, length, flags);
}

std::size_t Socket::recv(void *buffer, int length, int flags) {
    assert(valid());
    ssize_t ret = ::recv(socket_, buffer, length, flags);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        throwErrno();
    }
    return ret;
}

ssize_t Socket::sendNoExcept(void *buffer, int length, int flags) noexcept {
    assert(valid());
    return ::send(socket_, buffer, length, flags);
}

void Socket::send(void *buffer, int length, int flags) {
    assert(valid());
    ssize_t ret = ::send(socket_, buffer, length, flags);
    if (ret == -1) {
        throwErrno();
    }
    if (ret != length) {
        throw std::runtime_error("send() did not send all bytes (sent " +
                                 std::to_string(ret) + ", requested " +
                                 std::to_string(length) + ")");
    }
}

void Socket::setsockopt(int level, int option_name, const void *option_value,
                        SocketLen_t option_len) {
    if (::setsockopt(socket_, level, option_name, option_value, option_len) ==
        -1) {
        throwErrno();
    }
}

} // namespace lt::os

#endif
