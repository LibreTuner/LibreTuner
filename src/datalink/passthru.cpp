//
// Created by altenius on 12/20/18.
//

#include "passthru.h"
#include "j2534/j2534caninterface.h"

namespace datalink {
    PassThruLink::PassThruLink(j2534::Info &&info) : Link(info.name), info_(std::move(info)) {

    }

    void PassThruLink::check_interface() {
        if (!interface_) {
            interface_ = j2534::J2534::create(j2534::Info(info_));
            interface_->init();
        }
    }

    std::unique_ptr<CanInterface> PassThruLink::can(uint32_t baudrate) {
        check_device();
        return std::make_unique<j2534::Can>(device_, baudrate);
    }

    void PassThruLink::check_device() {
        if (!device_) {
            check_interface();
            if (port_.empty())
                device_ = interface_->open();
            else
                device_ = interface_->open(port_.c_str());
            if (!device_) {
                throw std::runtime_error("Failed to create PassThru device.");
            }
        }
    }

    Protocol PassThruLink::supported_protocols() const {
        return info_.protocols;
    }

    std::vector<std::unique_ptr<PassThruLink>> detect_passthru_links() {
        std::vector<j2534::Info> info = j2534::detect_interfaces();

        std::vector<std::unique_ptr<PassThruLink>> links;

        for (j2534::Info &i : info) {
            links.emplace_back(std::make_unique<datalink::PassThruLink>(std::move(i)));
        }

        return links;
    }
}

