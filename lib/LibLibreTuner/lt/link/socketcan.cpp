#ifdef WITH_SOCKETCAN

#include "socketcan.h"

#include <utility>

#include "../network/can/socketcan.h"

namespace lt {
SocketCanLink::SocketCanLink(const std::string &name, std::string device)
    : DataLink(name), device_(std::move(device)) {}

network::CanPtr SocketCanLink::can(uint32_t /*baudrate*/) {
    return std::make_unique<network::SocketCan>(device_);
}

DataLinkFlags SocketCanLink::flags() const noexcept { return DataLinkFlags::Port; }

DataLinkPortType SocketCanLink::portType() const {
    return DataLinkPortType::NetworkCan;
}

} // namespace lt

#endif
