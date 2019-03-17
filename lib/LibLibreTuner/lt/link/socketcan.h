//
// Created by altenius on 12/20/18.
//

#ifndef LT_SOCKETCAN_H
#define LT_SOCKETCAN_H

#ifdef WITH_SOCKETCAN

#include "datalink.h"

namespace lt {
    class SocketCanLink : public DataLink {
    public:
        SocketCanLink(const std::string &name, const std::string &device);

        DataLinkType type() const override { return DataLinkType::SocketCan; }

        network::CanPtr can(uint32_t baudrate) override;

        NetworkProtocol supportedProtocols() const override { return NetworkProtocol::Can; }
        
        std::string port() const override { return device_; }

    private:
        // std::shared_ptr<SocketCanInterface> interface_;
        std::string device_;

        // void check_interface();
    };
}

#endif


#endif //LT_SOCKETCAN_H
