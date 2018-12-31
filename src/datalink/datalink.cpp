//
// Created by altenius on 12/20/18.
//

#include "datalink.h"
#include "protocols/caninterface.h"
#include "protocols/isotpprotocol.h"

datalink::Link::Link(const std::string &name) : name_(name) {

}

std::unique_ptr<isotp::Protocol> datalink::Link::isotp() {
    return nullptr;
}

std::unique_ptr<CanInterface> datalink::Link::can(uint32_t baudrate) {
    return nullptr;
}
