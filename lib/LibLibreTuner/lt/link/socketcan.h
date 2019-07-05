#ifndef LT_SOCKETCAN_H
#define LT_SOCKETCAN_H

#ifdef WITH_SOCKETCAN

#include "datalink.h"

namespace lt
{
class SocketCanLink : public DataLink
{
public:
    SocketCanLink(const std::string & name, std::string device);

    DataLinkType type() const override { return DataLinkType::SocketCan; }

    network::CanPtr can(uint32_t baudrate) override;

    NetworkProtocol supportedProtocols() const override
    {
        return NetworkProtocol::Can;
    }

    std::string port() const override { return device_; }

    void setPort(const std::string & port) noexcept override { device_ = port; }

    // Supports port (network can)
    DataLinkFlags flags() const noexcept override;

    DataLinkPortType portType() const override;

private:
    std::string device_;

    // void check_interface();
};
} // namespace lt

#endif

#endif // LT_SOCKETCAN_H
