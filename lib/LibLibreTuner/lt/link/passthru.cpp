#include "passthru.h"
#include "../network/can/j2534can.h"
#include "../support/types.h"
#include <network/isotp/isotpj2534.h>

namespace lt
{
NetworkProtocol passthruToNetworkProtocol(j2534::Protocol passthru)
{
    NetworkProtocol protocol{NetworkProtocol::None};
    if ((passthru & j2534::Protocol::CAN) != j2534::Protocol::None)
        protocol |= NetworkProtocol::Can;
    if ((passthru & j2534::Protocol::ISO15765) != j2534::Protocol::None)
        protocol |= NetworkProtocol::IsoTp;
    return protocol;
}

PassThruLink::PassThruLink(j2534::Info && info)
    : DataLink(info.name), info_(std::move(info))
{
}

void PassThruLink::checkInterface()
{
    if (interface_)
        return;

    interface_ = j2534::J2534::create(j2534::Info(info_));
    interface_->init();
}

network::CanPtr PassThruLink::can(uint32_t baudrate)
{
    return std::make_unique<network::J2534Can>(getDevice(), baudrate);
}

j2534::DevicePtr PassThruLink::getDevice()
{
    if (auto device = device_.lock())
        return device;

    j2534::DevicePtr device;

    checkInterface();
    if (port_.empty())
        device = interface_->open();
    else
        device = interface_->open(port_.c_str());

    if (!device)
        throw std::runtime_error("Failed to create PassThru device.");

    device_ = device;
    return device;
}

NetworkProtocol PassThruLink::supportedProtocols() const
{
    return passthruToNetworkProtocol(info_.protocols);
}

DataLinkFlags PassThruLink::flags() const noexcept
{
    return DataLinkFlags::Port;
}

network::IsoTpPtr PassThruLink::isotp(const network::IsoTpOptions & options) {
    if ((info_.protocols & j2534::Protocol::ISO15765) != j2534::Protocol::None)
        return std::make_unique<network::IsoTpJ2534>(getDevice(), options);
    // Fall back to our ISO-TP stack
    return DataLink::isotp(options);
}

std::vector<std::unique_ptr<PassThruLink>> detect_passthru_links()
{
    std::vector<j2534::Info> info = j2534::detect_interfaces();

    std::vector<std::unique_ptr<PassThruLink>> links;

    links.reserve(info.size());
    for (j2534::Info & i : info)
    {
        links.emplace_back(std::make_unique<PassThruLink>(std::move(i)));
    }

    return links;
}
} // namespace lt
