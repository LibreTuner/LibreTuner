/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "socketcaninterface.h"

#include <cassert>
#include <cstring>
#include <unordered_map>

#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>

#include "libretuner.h"

static std::unordered_map<std::string, std::weak_ptr<SocketCanInterface>> cache;

SocketCanInterface::~SocketCanInterface() {
    if (socket_ != 0) {
        close();
    }
}



bool SocketCanInterface::recv(CanMessage &message,
                              std::chrono::milliseconds timeout) {
    can_frame frame;

    int nbytes = ::recv(socket_, &frame, sizeof(can_frame), 0);
    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            message.invalidate();
            return false;
        }
        throw std::runtime_error("Failed to read from socket: " +
                                 std::string(strerror(errno)));
    }

    // TODO: remove EFF/RTR/ERR flags
    message.setMessage(frame.can_id, frame.data, frame.can_dlc);
    return true;
}



void SocketCanInterface::send(const CanMessage &message) {
    can_frame frame;

    memset(&frame, 0, sizeof(can_frame));
    frame.can_dlc = message.length();
    frame.can_id = message.id();
    memcpy(frame.data, message.message(), message.length());

    if (int res =
            write(socket_, &frame, sizeof(can_frame)) != sizeof(can_frame)) {
        if (res < 0) {
            throw std::runtime_error("Failed to write to socket: " +
                                     std::string(strerror(errno)));
        }
        throw std::runtime_error(
            "Failed to write to socket: wrote fewer bytes than expected");
    }
}



bool SocketCanInterface::bind(const std::string &ifname) {
    socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_ == -1) {
        throw std::runtime_error("Failed to create socket: " +
                                 std::string(strerror(errno)));
    }

    sockaddr_can addr = {};
    ifreq ifr;

    strcpy(ifr.ifr_name, ifname.c_str());
    if (ioctl(socket_, SIOCGIFINDEX, &ifr) != 0) {
        throw std::runtime_error("Failed to find socketcan interface " + ifname + ": " +
                                 std::string(strerror(errno)));
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (::bind(socket_, (sockaddr *)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Failed to bind socketcan interface: " +
                                 std::string(strerror(errno)));
    }

    return true;
}



void SocketCanInterface::close() {
    assert(socket_ != 0);
    ::close(socket_);
    socket_ = 0;
}



void SocketCanInterface::start() { assert(socket_ != 0); }



int SocketCanInterface::fd() const { return socket_; }

SocketCanInterface::SocketCanInterface(const std::string &ifname) {
    assert(bind(ifname));
}



void SocketCanInterface::setNonblocking() {
    assert(socket_ != 0);
    int flags = fcntl(socket_, F_GETFL, 0);
    fcntl(socket_, F_SETFL, flags | O_NONBLOCK);
}
