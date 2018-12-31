//
// Created by altenius on 12/20/18.
//

#ifndef LIBRETUNER_SOCKETCAN_H
#define LIBRETUNER_SOCKETCAN_H

#include "protocols/socketcaninterface.h"
#include "datalink.h"

namespace datalink {
    class SocketCanLink : public Link {
    public:
        SocketCanLink(const std::string &name, const std::string &device);

        Type type() const override { return Type::SocketCan; }

        std::unique_ptr<CanInterface> can(uint32_t baudrate) override;

        Protocol supported_protocols() const override { return Protocol::Can; }

    private:
        // std::shared_ptr<SocketCanInterface> interface_;
        std::string device_;

        // void check_interface();
    };
}


#endif //LIBRETUNER_SOCKETCAN_H
