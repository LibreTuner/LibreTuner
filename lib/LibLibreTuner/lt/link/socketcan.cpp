//
// Created by altenius on 12/20/18.
//

#ifdef WITH_SOCKETCAN

#include "socketcan.h"

#include "../network/can/socketcan.h"

namespace lt {
    SocketCanLink::SocketCanLink(const std::string &name, const std::string &device) : DataLink(name), device_(device) {

    }

    network::CanPtr SocketCanLink::can(uint32_t _baudrate) {
        return std::make_unique<network::SocketCan>(device_);
    }
}

#endif
