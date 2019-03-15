//
// Created by altenius on 12/20/18.
//

#ifndef LT_SOCKETCAN_H
#define LT_SOCKETCAN_H

#ifdef WITH_SOCKETCAN

#include "protocols/socketcaninterface.h"
#include "datalink.h"

namespace datalink {
    class SocketCanLink : public Link {
    public:
        SocketCanLink(const std::string &name, const std::string &device);

        Type type() const override { return Type::SocketCan; }

        std::unique_ptr<CanInterface> can(uint32_t baudrate) override;

        Protocol supported_protocols() const override { return Protocol::Can; }
        
        std::string port() const override { return device_; }

    private:
        // std::shared_ptr<SocketCanInterface> interface_;
        std::string device_;

        // void check_interface();
    };
}

#endif


#endif //LT_SOCKETCAN_H
