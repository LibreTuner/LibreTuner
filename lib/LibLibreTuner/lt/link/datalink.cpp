#include <utility>

#include "datalink.h"

namespace lt {
DataLink::DataLink(std::string name) : name_(std::move(name)) {}

network::IsoTpPtr DataLink::isotp() { return nullptr; }

network::CanPtr DataLink::can(uint32_t /*baudrate*/) { return nullptr; }

} // namespace lt