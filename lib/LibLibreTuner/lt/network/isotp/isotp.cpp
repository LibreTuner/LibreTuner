#include "isotp.h"

namespace lt::network {

IsoTpPacket::IsoTpPacket() = default;

IsoTpPacket::IsoTpPacket(const uint8_t *data, size_t size)
    : data_(data, data + size) {}

void IsoTpPacket::setData(const uint8_t *data, size_t size) {
    data_.assign(data, data + size);
}

void IsoTpPacket::moveInto(std::vector<uint8_t> &data) {
    data = std::move(data_);
}

void IsoTpPacket::append(const uint8_t *data, size_t size) {
    data_.insert(data_.begin() + data_.size(), data, data + size);
}

} // namespace lt::network
