#include <utility>

#include "../network/isotp/isotpcan.h"
#include "datalink.h"

namespace lt
{
DataLink::DataLink(std::string name) : name_(std::move(name)) {}

network::IsoTpPtr DataLink::isotp(const network::IsoTpOptions & options)
{
    // Try to create a device from CAN
    network::CanPtr dev = can(500000);
    if (dev)
        return std::make_unique<network::IsoTpCan>(std::move(dev), options);
    return nullptr;
}

network::CanPtr DataLink::can(uint32_t /*baudrate*/) { return nullptr; }

} // namespace lt