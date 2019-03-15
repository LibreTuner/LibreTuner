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
    explicit DataLink(const std::string &name);
    virtual ~DataLink() = default;

    virtual DataLinkType type() const = 0;

    // Returns a bitflag of supported protocols
    virtual NetworkProtocol supportedProtocols() const = 0;

    // Returns the pretty name of the link for displaying to the user
    const std::string &name() const { return name_; }

    // Creates a CAN interface with the specified baudrate. The baudrate
    // may not be supported by the link. Returns nullptr if the interface is
    // not supported.
    virtual network::CanPtr can(uint32_t baudrate);

    // Currently a stub. Returns nullptr.
    virtual network::IsoTpPtr isotp();

    // Returns the port or an empty string if no port is used by the datalink
    // type
    virtual std::string port() const = 0;

protected:
    std::string name_;
};
using DataLinkPtr = std::unique_ptr<DataLink>;
} // namespace lt

#endif // LT_DATALINK_H
