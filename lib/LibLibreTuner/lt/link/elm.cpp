#include <utility>

#include "elm.h"

#include "../network/isotp/isotpelm.h"

namespace lt
{

ElmDataLink::ElmDataLink(const std::string & name, std::string port,
                         int baudrate)
    : DataLink(name), port_(std::move(port)), uartBaudrate_{baudrate}
{
}

NetworkProtocol ElmDataLink::supportedProtocols() const
{
    return NetworkProtocol::IsoTp;
}

DataLinkType ElmDataLink::type() const { return DataLinkType::Elm; }

network::CanPtr ElmDataLink::can(uint32_t /*baudrate*/)
{
    // TODO: Look into CAN support with USER1 & USER2
    return nullptr;
}

network::Elm327Ptr ElmDataLink::createDevice()
{
    if (auto device = device_.lock(); device && device->isOpen())
        return device;

    serial::Settings settings;
    settings.baudrate = uartBaudrate_;

    network::Elm327Ptr device = std::make_shared<network::Elm327>(port_, settings);
    device->open();
    device_ = device;
    return device;
}

void ElmDataLink::setPort(const std::string & port) { port_ = port; }

network::IsoTpPtr ElmDataLink::isotp(const network::IsoTpOptions & options)
{
    return std::make_unique<network::IsoTpElm>(createDevice(), options);
}

int ElmDataLink::baudrate() { return uartBaudrate_; }

void ElmDataLink::setBaudrate(int baudrate) { uartBaudrate_ = baudrate; }

DataLinkFlags ElmDataLink::flags() const noexcept
{
    return DataLinkFlags::Port | DataLinkFlags::Baudrate;
}

std::vector<std::string> ElmDataLink::ports()
{
    return serial::enumeratePorts();
}

} // namespace lt
