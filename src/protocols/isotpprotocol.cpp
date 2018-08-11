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

#include "isotpprotocol.h"
#include "timer.h"
#include "util.hpp"
#include "logger.h"

namespace isotp {
namespace details {

uint8_t calculate_st(std::chrono::microseconds time) {
  Expects(time.count() >= 0);
  if (time.count() == 0) {
    return 0;
  }

  if (time >= std::chrono::milliseconds(1)) {
    return std::min<long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(time).count(),
        127);
  } 
    uint8_t count = std::max<uint8_t>(time.count() / 100, 1);
    return count + 0xF0;
  
}

std::chrono::microseconds calculate_time(uint8_t st) {
  if (st <= 127) {
    return std::chrono::milliseconds(st);
  }
  if (st >= 0xF1 && st <= 0xF9) {
    return std::chrono::microseconds((st - 0xF0) * 100);
  }
  return std::chrono::microseconds(0);
}

FrameType frame_type(const CanMessage &message) {
  if (message.length() == 0) {
    return FrameType::None;
  }

  uint8_t beg = message[0] & 0xF0;
  if (beg > 3) {
    return FrameType::None;
  }
  return static_cast<FrameType>(beg);
}

} // namespace details

Frame Frame::single(gsl::span<uint8_t> data) {
  Expects(data.size() <= 7);
  Frame f;
  f.data[0] = data.size();
  f.length = data.size() + 1;
  std::copy(data.begin(), data.end(), f.data.begin() + 1);
  return f;
}

Frame Frame::first(uint16_t size, gsl::span<uint8_t, 6> data) {
  Frame f;
  f.data[0] = 0x10 | ((size >> 8) & 0x0F);
  f.data[1] = static_cast<uint8_t>(size & 0xFF);
  f.length = data.size() + 2;
  // Copy the data into the frame
  std::copy(data.begin(), data.end(), f.data.begin() + 2);
  return f;
}

Frame Frame::consecutive(uint8_t index, gsl::span<uint8_t> data) {
  Expects(data.size() <= 7);
  Expects(index <= 15);
  Frame f;
  // CAN-TP Header
  f.data[0] = 0x20 | index;
  f.length = data.size() + 1;
  // Copy the data into the frame
  std::copy(data.begin(), data.end(), f.data.begin() + 1);
  return f;
}

Frame Frame::flow(const FlowControlFrame &frame) {
  Frame f;
  f.data[0] = 0x30 | static_cast<uint8_t>(frame.flag);
  f.data[1] = frame.blockSize;
  f.data[2] = details::calculate_st(frame.separationTime);
  f.length = 3;
  return f;
}

uint8_t Protocol::incConsec() {
  if (++consecIndex_ == 16) {
    consecIndex_ = 0;
  }
  return consecIndex_;
}

void Protocol::error(Error err) {
  timer_->disable();
  callback_(err, std::move(packet_));
}

void Protocol::received() { error(Error::Success); }

bool Protocol::finishRead(uint8_t amount) {
  if (amount > remaining_) {
    error(Error::Unknown);
    return false;
  }
  remaining_ -= amount;
  if (remaining_ == 0) {
    received();
    return false;
  }
  timer_->enable();
  return true;
}

void Protocol::handleFirst(const FirstFrame &f) {
  remaining_ = f.size;
  size_t ff_length = std::min<size_t>(f.data_length, remaining_);
  packet_.append(gsl::make_span(f.data).first(ff_length));
  state_ = State::Consecutive;
  if (finishRead(ff_length)) {
    protocol_.sendFlowFrame(FlowControlFrame());
  }
}

void Protocol::handleSingle(const SingleFrame &f) {
  packet_.append(gsl::make_span(f.data).first(f.size));
  received();
}

void Protocol::handleBegin(const Frame &f) {
  FirstFrame ff{};
  SingleFrame sf{};
  if (f.first(ff)) {
    handleFirst(ff);
  } else if (f.single(sf)) {
    handleSingle(sf);
  }
}

void Protocol::handleConsec(const ConsecutiveFrame &f) {
  if (f.index != incConsec()) {
    error(Error::Consec);
    return;
  }
  size_t cf_length = std::min<size_t>(f.data_length, remaining_);
  packet_.append(gsl::make_span(f.data).first(cf_length));
  finishRead(cf_length);
}

void Protocol::recv(Protocol::RecvPacketCallback cb) {
  std::shared_ptr<Receiver> receiver =
      std::make_shared<make_shared_enabler<Receiver>>(protocol, std::move(cb));
}

void Protocol::start() {
  timer_->enable();
  connection_ = protocol_.listen([&](const Frame &f) {
    timer_->disable();
    switch (state_) {
    case State::Begin:
      if (f.type() == FrameType::Single || f.type() == FrameType::First)
        handleBegin(f);
      break;
    case State::Consecutive:
      if (f.type() == FrameType::Consecutive) {
        ConsecutiveFrame cf{};
        if (!f.consecutive(cf)) {
          break;
        }
        handleConsec(cf);
      }
      break;
    }
  });
}


void Protocol::notifySendResult(Error error) {
    Logger::debug("Calling callback");
    callback_(error);
}

bool Protocol::recvFrame(Frame &frame)
{
    while (true) {
        CanMessage msg;
        if (!can_->recv(msg, options_.timeout)) {
            return false;
        }

        if (msg.length() == 0 || msg.id() != options_.destId) {
          continue;
        }
        frame = Frame(msg);
        return true;
    }
}

void Protocol::waitForControlFrame() {
    timer_->setCallback([this] {
        notifySendResult(Error::Timeout);
    });
    timer_->enable();

    Frame frame;

    if (!recvFrame(frame)) {
      // TIMED OUT
      return;
    }

    FlowControlFrame fc;
    if (!frame.flow(fc)) {
      return;
    }
    timer_->disable();
    onFlow(fc);
}

uint8_t Sender::incrementConsec() {
  if (++consecIndex_ == 16) {
    consecIndex_ = 0;
  }
  return consecIndex_;
}

void Protocol::onFlow(FlowControlFrame &frame) {
  if (consecThread_.joinable()) {
    consecThread_.join();
  }
  // Send consecutive frames
  if (frame.flag == FCFlag::Overflow) {
    callback_(Error::Unknown);
    return;
  }
  if (frame.flag == FCFlag::Wait) {
    waitForControlFrame();
    return;
  }

  consecIndex_ = 0;

  if (frame.separationTime.count() == 0) {
    frame.separationTime = std::chrono::microseconds(100);
  }

  consecThread_ = std::thread([this, frame] {
    try {
    uint8_t bs = frame.blockSize;
    while (!packet_.eof()) {
      if (frame.separationTime.count() != 0)
        std::this_thread::sleep_for(frame.separationTime);
      std::vector<uint8_t> data = packet_.next(7);
      protocol_.sendConsecutiveFrame(incrementConsec(), data);

      if (bs > 0) {
        if (bs == 1) {
          if (!packet_.eof()) {
            waitForControlFrame();
            return;
          }
          break;
        }
        bs--;
      }
    }
    notifySendResult(Error::Success);
      } catch (const std::exception &e) {
          Logger::critical("Error in isotp consecutive thread: " + std::string(e.what()));
          notifySendResult(Error::Unknown);
      }
  });
}

Frame::Frame(const CanMessage &message) {
  std::copy(message.message(), message.message() + message.length(),
            data.begin());
  length = message.length();
}

FrameType Frame::type() const {
  if (length == 0) {
    return FrameType::None;
  }

  uint8_t beg = (data[0] & 0xF0) >> 4;
  if (beg > 3) {
    return FrameType::None;
  }
  return static_cast<FrameType>(beg);
}

bool Frame::single(SingleFrame &f) const {
  if (type() != FrameType::Single) {
    return false;
  }

  uint8_t size = data[0] & 0x0F;
  if (length < size + 1) {
    return false;
  }

  // then this is a valid frame
  f.size = size;
  std::copy(data.begin() + 1, data.begin() + size + 1, f.data.begin());
  return true;
}

bool Frame::first(FirstFrame &f) const {
  if (type() != FrameType::First) {
    return false;
  }

  // Check if the frame is at least two bytes to fit the type and size header
  if (length < 2) {
    return false;
  }

  f.size = ((data[0] & 0x0F) << 8) | (data[1]);
  f.data_length = length - 2;
  std::copy(data.begin() + 2, data.begin() + length, f.data.begin());
  return true;
}

bool Frame::consecutive(ConsecutiveFrame &f) const {
  if (type() != FrameType::Consecutive) {
    return false;
  }

  f.index = data[0] & 0x0F;
  f.data_length = length - 1;
  std::copy(data.begin() + 1, data.begin() + length, f.data.begin());
  return true;
}

bool Frame::flow(FlowControlFrame &f) const {
  if (type() != FrameType::Flow) {
    return false;
  }

  if (length < 3) {
    return false;
  }

  uint8_t flag = data[0] & 0x0F;
  // Check that the flag is in the valid range (0, 1, 2)
  if (flag > 2) {
    return false;
  }

  f.flag = static_cast<FCFlag>(flag);
  f.blockSize = data[1];
  f.separationTime = details::calculate_time(data[2]);
  return true;
}

Packet::Packet() { pointer_ = std::begin(data_); }

Packet::Packet(gsl::span<const uint8_t> data) { setData(data); }

void Packet::moveAll(std::vector<uint8_t> &data) {
  data = std::move(data_);
  pointer_ = std::begin(data_);
}

void Packet::append(gsl::span<const uint8_t> data) {
  auto pOffset = std::distance(std::begin(data_), pointer_);
  data_.insert(data_.end(), data.begin(), data.end());
  pointer_ = std::begin(data_) + pOffset;
}

std::vector<uint8_t> Packet::next(size_t max) {
  size_t amount = std::min<size_t>(std::end(data_) - pointer_, max);
  auto begin = pointer_;
  pointer_ += amount;
  return std::vector<uint8_t>(begin, pointer_);
}

uint8_t Packet::next() { return *(pointer_++); }

void Packet::setData(gsl::span<const uint8_t> data) {
  data_.assign(data.begin(), data.end());
  pointer_ = std::begin(data_);
}

Protocol::Protocol(std::unique_ptr<CanInterface> &&can, Options options)
    : options_(options) {
    setCan(std::move(can));
}

Protocol::~Protocol()
{
    Logger::debug("Destructed protocol");
}

void Protocol::setCan(std::unique_ptr<CanInterface> &&can) {
  can_ = std::move(can);
}

void Protocol::request(Packet &&req, Protocol::RecvPacketCallback &&cb) {
    try {
  send(std::move(req), [this, cb{std::move(cb)}](Error error) mutable {
        if (error != Error::Success) {
          cb(error, Packet());
          return;
        }
        recvPacketAsync(std::move(cb));
      });
    } catch (const std::exception &e) {
        Logger::critical("Error while sending ISO-TP request: " + std::string(e.what()));
        //cb(Error::Unknown, Packet());
    }
}

void Protocol::send(const Frame &frame) {
  Expects(can_);
  can_->send(options_.sourceId, frame.data);
}

void Protocol::sendSingleFrame(gsl::span<uint8_t> data) {
  send(Frame::single(data));
}

void Protocol::sendFirstFrame(uint16_t size, gsl::span<uint8_t, 6> data) {
  send(Frame::first(size, data));
}

void Protocol::sendConsecutiveFrame(uint8_t index, gsl::span<uint8_t> data) {
  send(Frame::consecutive(index, data));
}

void Protocol::sendFlowFrame(const FlowControlFrame &fc) {
  send(Frame::flow(fc));
}

void Protocol::send(Packet &&packet, SendPacketCallback &&cb) {
    try {
      if (packet.size() <= 7) {
          std::vector<uint8_t> data = packet.next(7);
          sendSingleFrame(data);
          Logger::info("Sent single frame");
          notifySendResult(Error::Success);
      } else {
          waitForControlFrame();
          auto remaining = packet.remaining();
          std::vector<uint8_t> data = packet.next(6);
          sendFirstFrame(remaining, data);
      }
     } catch (const std::exception &e) {
          Logger::critical("Failed to send ISO-TP message: " + std::string(e.what()));
      }
}

void Protocol::recvPacketAsync(Protocol::RecvPacketCallback &&cb) {
  Receiver::recv(*this, std::move(cb));
}

std::string strError(Error error) {
  switch (error) {
  case Error::Success:
    return "success";
  case Error::Unknown:
    return "unknown";
  case Error::Consec:
    return "invalid consecutive index for frame";
  case Error::Timeout:
    return "request timed out";
  default:
    return "unknown";
  }
}

} // namespace isotp
