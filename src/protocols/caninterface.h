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

#ifndef CANINTERFACE_H
#define CANINTERFACE_H

#include "util/signal.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <gsl/span>

struct CanMessage {
public:
  void setMessage(uint32_t id, gsl::span<const uint8_t> data);

  bool valid() const { return messageLength_ != 0; }

  void invalidate() { messageLength_ = 0; }

  CanMessage();
  CanMessage(uint32_t id, gsl::span<const uint8_t> data);

  uint32_t id() const { return id_; }

  const uint8_t *message() const { return message_; }

  /* Returns a human-readable string representing the message data */
  std::string strMessage() const;

  uint8_t length() const { return messageLength_; }

  uint8_t &operator[](uint8_t index) { return message_[index]; }

  const uint8_t &operator[](uint8_t index) const { return message_[index]; }

  uint32_t id_;
  uint8_t message_[8];
  uint8_t messageLength_;
};

class CanInterface;
typedef std::shared_ptr<CanInterface> CanInterfacePtr;

/* Abstract CAN interface */
class CanInterface {
public:
  enum class CanError {
    Success = 0,
    Socket, // Socket creation error. err will be set
    Read,   // Read error. err will be set
    Write,
  };

  using Listener = std::function<void(const CanMessage &message)>;
  using SignalType = Signal<Listener>;

  CanInterface();
  virtual ~CanInterface() = default;

  /* Send a CAN message. The size of data must be <= 8
   * Returns true if a message was sent */
  void send(int id, gsl::span<const uint8_t> data);

  virtual void send(const CanMessage &message) = 0;

  /* Connects a new listener */
  std::shared_ptr<SignalType::ConnectionType> connect(Listener listener) {
    return signal_->connect(std::move(listener));
  }

  /* Returns true if the socket is ready for reading/writing */
  virtual bool valid() = 0;

  /* Starts reading from the interface and calling callbacks. This function
   * may block until the the interface has fully started. */
  virtual void start() = 0;

protected:
  std::shared_ptr<SignalType> signal_;
  CanError lastError_;
  int lastErrno_;
};

#endif // CANINTERFACE_H
