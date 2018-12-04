/*
 * LibreTuner{}{}
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "caninterface.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <sstream>

void CanMessage::setMessage(uint32_t id, const uint8_t *data, size_t size) {
    assert(size <= 8);
    id_ = id;
    messageLength_ = static_cast<uint8_t>(size);
    std::copy(data, data + size, message_);
}

std::string CanMessage::strMessage() const {
    std::stringstream ss;
    for (uint8_t c = 0; c < length(); ++c) {
        if (c != 0) {
            ss << "  ";
        }
        ss << std::hex << static_cast<unsigned int>(message_[c]);
    }

    return ss.str();
}

CanMessage::CanMessage() : id_(0), message_{0}, messageLength_(0) {}

CanMessage::CanMessage(uint32_t id, const uint8_t *data, size_t size) {
    setMessage(id, data, size);
}

// CanInterface::CanInterface() : signal_(SignalType::create()) {}
CanInterface::CanInterface() {}

void CanInterface::send(int id, const uint8_t *data, size_t size) {
    send(CanMessage(id, data, size));
}
