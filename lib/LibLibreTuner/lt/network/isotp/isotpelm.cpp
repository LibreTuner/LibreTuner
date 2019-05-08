#include "isotpelm.h"

#include <cassert>

namespace lt::network {
IsoTpElm::IsoTpElm(Elm327Ptr device, IsoTpOptions options) : device_(std::move(device)), options_(options) {
    assert(device_);
    if (!device_->isOpen()) {
        throw std::runtime_error("Elm327 device is not open");
    }

    // Set protocol
    device_->setProtocol(ElmProtocol::ISO_15765_4_CAN_11bit_500);
    // Disable printing spaces
    device_->setPrintSpaces(false);

    updateOptions();
}

void IsoTpElm::recv(IsoTpPacket &result) {

}

void IsoTpElm::request(const IsoTpPacket &req, IsoTpPacket &result) {

}

void IsoTpElm::send(const IsoTpPacket &packet) {

}

void IsoTpElm::setOptions(const IsoTpOptions &options) {
    options_ = options;
    updateOptions();
}

void IsoTpElm::updateOptions() {
    device_->setHeader(options_.sourceId);
    device_->setCanReceiveAddress11(options_.destId);
    // TODO: set timeout
}
}