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
    
SocketCan::SocketCan(const std::string &ifname) : socket_(PF_CAN, SOCK_RAW, CAN_RAW) {
    sockaddr_can addr = {};
    ifreq ifr;

    std::strcpy(ifr.ifr_name, ifname.c_str());
    socket_.ioctl(SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    socket_.bind(reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    
    // Set timeout to 1s
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    socket_.setsockopt(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
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
    can_frame frame;

    int nbytes = socket_.recv(&frame, sizeof(can_frame), 0);
    if (nbytes == 0) {
        // Timed out
        return false;
    }

    // TODO: remove EFF/RTR/ERR flags
    message.setMessage(frame.can_id, frame.data, frame.can_dlc);
    return true;
}
    
} // namespace network
} // namespace lt

#endif