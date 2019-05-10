#include <utility>

#include "elm.h"

#include "../network/isotp/isotpelm.h"

namespace lt {

ElmDataLink::ElmDataLink(const std::string &name, std::string port) : DataLink(name), port_(std::move(port)) {

}

NetworkProtocol ElmDataLink::supportedProtocols() const {
    return NetworkProtocol::IsoTp;
}

DataLinkType ElmDataLink::type() const {
    return DataLinkType::Elm;
}

network::CanPtr ElmDataLink::can(uint32_t baudrate) {
    // TODO: Look into CAN support with USER1 & USER2
    return nullptr;
}

void ElmDataLink::createDevice() {
    if (device_) {
        return;
    }
    device_ = std::make_shared<network::Elm327>();
    serial::Device &device = device_->device();
    device.setPort(port_);

    serial::Settings settings;
    settings.baudrate = uartBaudrate_;
    device.setSettings(settings);

    device_->open();
}

void ElmDataLink::setPort(const std::string &port) {
    port_ = port;
}

network::IsoTpPtr ElmDataLink::isotp(const network::IsoTpOptions &options) {
    createDevice();

    return std::make_unique<network::IsoTpElm>(device_, options);
}

}
