//
// Created by altenius on 12/20/18.
//

#ifndef LT_DATALINK_H
#define LT_DATALINK_H

#include <memory>
#include <string>

#include "../network/network.h"
#include "../support/types.h"

namespace lt {
class DataLink {
public:
    explicit DataLink(std::string name);
    virtual ~DataLink() = default;

    virtual DataLinkType type() const = 0;

    // Returns a bitflag of supported protocols
    virtual NetworkProtocol supportedProtocols() const = 0;

    // Returns the pretty name of the link for displaying to the user
    inline const std::string &name() const noexcept { return name_; }

    inline void setName(const std::string &name) noexcept { name_ = name; }

    // Creates a CAN interface with the specified baudrate. The baudrate
    // may not be supported by the link. Returns nullptr if the interface is
    // not supported.
    virtual network::CanPtr can(uint32_t baudrate);

    // Tries to create a device wrapped around can()
    virtual network::IsoTpPtr isotp(const network::IsoTpOptions &options);

    // Returns the port or an empty string if no port is used by the datalink
    // type
    virtual std::string port() const = 0;
    virtual void setPort(const std::string &port) =0;

protected:
    std::string name_;
};
using DataLinkPtr = std::unique_ptr<DataLink>;
} // namespace lt

#endif // LT_DATALINK_H
