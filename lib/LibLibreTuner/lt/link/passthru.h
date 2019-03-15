//
// Created by altenius on 12/20/18.
//

#ifndef LT_PASSTHRU_H
#define LT_PASSTHRU_H

#include "../j2534/j2534.h"
#include "datalink.h"

#include <memory>
#include <vector>

namespace lt {
class PassThruLink : public DataLink {
public:
    explicit PassThruLink(j2534::Info &&info);

    DataLinkType type() const override { return DataLinkType::PassThru; };

    void setPort(const std::string &port) { port_ = port; }

    std::string port() const override { return port_; }

    NetworkProtocol supportedProtocols() const override;

    network::CanPtr can(uint32_t baudrate) override;

private:
    std::string port_;
    // device_ must be destructed before interface_
    j2534::DevicePtr device_;
    j2534::J2534Ptr interface_;
    j2534::Info info_;

    void checkInterface();
    void checkDevice();
};
using PassThruLinkPtr = std::unique_ptr<PassThruLink>;

// Returns a vector of all installed PassThru drivers
std::vector<PassThruLinkPtr> detect_passthru_links();
} // namespace lt

#endif // LT_PASSTHRU_H
