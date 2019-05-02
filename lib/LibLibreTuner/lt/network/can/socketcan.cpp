#include "socketcan.h"

#ifdef WITH_SOCKETCAN

#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>

#include <cstring>

namespace lt {
namespace network {
    
bool SocketCanReceiver::recv(CanMessage &message, std::chrono::milliseconds timeout) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while ((std::chrono::steady_clock::now() - start) < timeout) {
        if (!running_) {
            if (result_.valid()) {
                result_.get();
            }
            // If the result did not throw an exception, it was stop()'d
            throw std::runtime_error("SocketCAN receiver thread is inactive");
        }
        
        if (buffer_.pop(message)) {
            return true;
        }
        
        std::unique_lock lk(mutex_);
        received_.wait_for(lk, timeout - (std::chrono::steady_clock::now() - start));
    }
    // Timed out
    return false;
}

SocketCanReceiver::~SocketCanReceiver() {
    stop();
}

void SocketCanReceiver::work() {
    while (!stop_) {
        can_frame frame;

        int nbytes = socket_.recv(&frame, sizeof(can_frame), 0);
        if (nbytes == 0) {
            // Timed out
            continue;
        }

        // TODO: remove EFF/RTR/ERR flags
        buffer_.add(CanMessage(frame.can_id, frame.data, frame.can_dlc));
        received_.notify_all();
    }
}

void SocketCanReceiver::stop() {
    if (!running_) {
        return;
    }
    stop_ = true;
    receiver_.join();
}

void SocketCanReceiver::start() {
    if (running_) {
        return;
    }
    
    stop_ = false;
    running_ = true;
    receiver_ = std::thread([this]() {
        std::packaged_task<void()> task([this]() {
            work();
        });
        
        result_ = task.get_future();
        task();
        running_ = false;
    });
}

void SocketCanReceiver::clearBuffer() {
    buffer_.clear();
}

SocketCan::~SocketCan() {
    
}
    
SocketCan::SocketCan(const std::string &ifname) : socket_(PF_CAN, SOCK_RAW, CAN_RAW), receiver_(socket_) {
    sockaddr_can addr = {};
    ifreq ifr;

    std::strcpy(ifr.ifr_name, ifname.c_str());
    socket_.ioctl(SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    socket_.bind(reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    
    // Set timeout to 1s
    timeval tv = {0};
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    socket_.setsockopt(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    receiver_.start();
}
    

    
void SocketCan::send(const CanMessage &message) {
    can_frame frame = {0};
    
    frame.can_dlc = message.length();
    frame.can_id = message.id();
    std::copy(message.message(), message.message() + message.length(), frame.data);
    
    socket_.send(&frame, sizeof(can_frame), 0);
}



bool SocketCan::recv(CanMessage &message,
                      std::chrono::milliseconds timeout) {
    return receiver_.recv(message, timeout);
}
    
void SocketCan::clearBuffer() noexcept {
    receiver_.clearBuffer();
}
    
} // namespace network
} // namespace lt

#endif
