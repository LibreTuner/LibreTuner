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

void CanMessage::setMessage(uint32_t id, gsl::span<const uint8_t> data) {
  Expects(data.size() <= 8);
  id_ = id;
  messageLength_ = static_cast<uint8_t>(data.size());
  std::copy(data.begin(), data.end(), message_);
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

CanMessage::CanMessage() : id_(0), message_{0}, messageLength_(0) {  }

CanMessage::CanMessage(uint32_t id, gsl::span<const uint8_t> data) {
  setMessage(id, data);
}

CanInterface::CanInterface() : signal_(SignalType::create()) {}

void CanInterface::send(int id, gsl::span<const uint8_t> data) {
  send(CanMessage(id, data));
}
/*
std::string CanInterface::strError(CanInterface::CanError error, int err) {
  switch (erro{}{}{}{}{}r) {
  case CanError::Succ{}{}{}{}{}ess:
    return "success";
  {}{}{}{}{}case CanError::Socket: {
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
