#ifndef LT_ELM_DATALINK_H
#define LT_ELM_DATALINK_H

#include "../network/command/elm327.h"
#include "datalink.h"

namespace lt {

class ElmDataLink : public DataLink {
public:
    explicit ElmDataLink(const std::string &name, std::string port = "");

    DataLinkType type() const override;

    NetworkProtocol supportedProtocols() const override;

    // Stub. Returns nullptr
    network::CanPtr can(uint32_t baudrate) override;

    network::IsoTpPtr isotp(const network::IsoTpOptions &options) override;

    std::string port() const override { return port_; }

    void setPort(const std::string &port) override;

    // If the device has not been initialized, initializes it
    void createDevice();

private:
    std::string port_;
    network::Elm327Ptr device_;
    int uartBaudrate_{38400};
};

} // namespace lt

#endif // LT_ELM_DATALINK_H
