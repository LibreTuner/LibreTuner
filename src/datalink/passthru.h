//
// Created by altenius on 12/20/18.
//

#ifndef LIBRETUNER_PASSTHRU_H
#define LIBRETUNER_PASSTHRU_H

#include "datalink.h"
#include "j2534/j2534.h"

#include <vector>
#include <memory>

namespace datalink {
    class PassThruLink : public Link {
    public:
        explicit PassThruLink(j2534::Info &&info);

        Type type() const override { return Type::PassThru; };

        void set_port(const std::string &port) { port_ = port; }

        const std::string &port() const { return port_; }

        Protocol supported_protocols() const override;

        std::unique_ptr<CanInterface> can(uint32_t baudrate) override;

    private:
        std::string port_;
        // device_ must be destructed before interface_
        j2534::DevicePtr device_;
        j2534::J2534Ptr interface_;
        j2534::Info info_;


        void check_interface();
        void check_device();
    };

    // Returns a vector of all installed PassThru drivers
    std::vector<std::unique_ptr<PassThruLink>> detect_passthru_links();
}


#endif //LIBRETUNER_PASSTHRU_H
