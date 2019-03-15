//
// Created by altenius on 12/20/18.
//

#include "socketcan.h"

#ifdef WITH_SOCKETCAN

namespace datalink {
    SocketCanLink::SocketCanLink(const std::string &name, const std::string &device) : Link(name), device_(device) {

    }

    std::unique_ptr<CanInterface> SocketCanLink::can(uint32_t _baudrate) {
        return std::make_unique<SocketCanInterface>(device_);
    }
}

#endif