/*
 * LibreTuner
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

void CanMessage::setMessage(uint32_t id, const uint8_t *message,
                            uint8_t length) {
  assert(length <= 8 && "Message length is too long!");
  id_ = id;
  messageLength_ = length;
  memcpy(message_, message, length);
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

CanMessage::CanMessage() { std::memset(message_, 0, 8); }

CanMessage::CanMessage(uint32_t id, const uint8_t *message, uint8_t length) {
  setMessage(id, message, length);
}

CanInterface::CanInterface() : signal_(SignalType::create()) {
  
}

void CanInterface::send(int id, const uint8_t *message, uint8_t len) {
  send(CanMessage(id, message, len));
}
/*
std::string CanInterface::strError(CanInterface::CanError error, int err) {
  switch (error) {
  case CanError::Success:
    return "success";
  case CanError::Socket: {
    std::stringstream ss;
    ss << "failed to create socket: " << strerror(err);
    return ss.str();
  }
  case CanError::Read: {
    std::stringstream ss;
    ss << "failed to read: " << strerror(err);
    return ss.str();
  }
  case CanError::Write:
    return "failed to transmit message";
  default:
    break;
  }

  return "unknown. You should not see this. If you do, please submit a pull "
         "request";
}*/


