//
// Created by altenius on 12/20/18.
//

#include "datalink.h"

namespace lt {
DataLink::DataLink(const std::string &name) : name_(name) {}

network::IsoTpPtr DataLink::isotp() { return nullptr; }

network::CanPtr DataLink::can(uint32_t baudrate) { return nullptr; }

} // namespace lt