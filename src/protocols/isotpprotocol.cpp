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

/* Used to receive ISO-TP messages */
// TODO: fix race conditions
class Receiver {
public:
  static void recv(Protocol &protocol, Protocol::RecvPacketCallback cb);

protected:
  Receiver(Protocol &protocol, Protocol::RecvPacketCallback &&cb);

private:
  void start();

  void handleFirst(const FirstFrame &f);

  void handleSingle(const SingleFrame &f);

  void handleBegin(const Frame &f);

  void handleConsec(const ConsecutiveFrame &f);

  enum class State {
    Begin, // Waiting for a Single or First frame
    Consecutive,
  };

  Packet packet_;
  Protocol &protocol_;
  Protocol::RecvPacketCallback callback_;
  uint8_t consecIndex_ = 0;
  // Remaining amount of bytes to be read
  uint16_t remaining_{};
  std::shared_ptr<Receiver> self_;
  State state_ = State::Begin;

  TimerPtr timer_;

  std::shared_ptr<Protocol::ConnectionType> connection_;

  void error(Error err);
  void received();
  // returns true if waiting on more data
  bool finishRead(uint8_t amount);
  void timedout();

  /* Increment the consecutive index, circling from 15 to 0 */
  uint8_t incConsec();
};

/* Used to send ISO-TP messages */
class Sender {
public:
  static void send(Protocol &protocol, Packet &&packet, Protocol::SendPacketCallback &&cb);
  ~Sender();

protected:
  Sender(Protocol &protocol, Packet &&packet, Protocol::SendPacketCallback &&cb);

private:
  void send();

  Packet packet_;
  Protocol &protocol_;
  Protocol::SendPacketCallback callback_;

  uint8_t consecIndex_ = 0;
  TimerPtr timer_;
  std::shared_ptr<Protocol::ConnectionType> connection_;
  std::thread consecThread_;

  void waitForControlFrame();
  void onFlow(FlowControlFrame &frame);
  uint8_t incrementConsec();
  void finish(Error error);

  std::shared_ptr<Sender> self_;
};

uint8_t Receiver::incConsec() {
  if (++consecIndex_ == 16) {
    consecIndex_ = 0;
  }
  return consecIndex_;
}

void Receiver::timedout() { error(Error::Timeout); }

void Receiver::error(Error err) {
  timer_->disable();
  connection_.reset();
  callback_(err, std::move(packet_));
  self_.reset();
}

void Receiver::received() { error(Error::Success); }

bool Receiver::finishRead(uint8_t amount) {
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

void Receiver::handleFirst(const FirstFrame &f) {
  remaining_ = f.size;
  size_t ff_length = std::min<size_t>(f.data_length, remaining_);
  packet_.append(gsl::make_span(f.data).first(ff_length));
  state_ = State::Consecutive;
  if (finishRead(ff_length)) {
    protocol_.sendFlowFrame(FlowControlFrame());
  }
}

void Receiver::handleSingle(const SingleFrame &f) {
  packet_.append(gsl::make_span(f.data).first(f.size));
  received();
}

void Receiver::handleBegin(const Frame &f) {
  FirstFrame ff{};
  SingleFrame sf{};
  if (f.first(ff)) {
    handleFirst(ff);
  } else if (f.single(sf)) {
    handleSingle(sf);
  }
}

void Receiver::handleConsec(const ConsecutiveFrame &f) {
  if (f.index != incConsec()) {
    error(Error::Consec);
    return;
  }
  size_t cf_length = std::min<size_t>(f.data_length, remaining_);
  packet_.append(gsl::make_span(f.data).first(cf_length));
  finishRead(cf_length);
}

void Receiver::recv(Protocol &protocol, Protocol::RecvPacketCallback cb) {
  std::shared_ptr<Receiver> receiver =
      std::make_shared<make_shared_enabler<Receiver>>(protocol, std::move(cb));
  receiver->self_ = receiver;
  receiver->start();
}

Receiver::Receiver(Protocol &protocol, Protocol::RecvPacketCallback &&cb)
    : protocol_(protocol), callback_(std::move(cb)), timer_(Timer::create(std::bind(&Receiver::timedout, this))) {
  packet_.clear();
  timer_->setTimeout(protocol_.options().timeout);
}

void Receiver::start() {
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

Sender::Sender(Protocol &protocol, Packet &&packet, Protocol::SendPacketCallback &&cb)
    : protocol_(protocol), packet_(std::move(packet)), timer_(Timer::create()), callback_(std::move(cb)) {
  timer_->setTimeout(protocol.options().timeout);
}

Sender::~Sender() {
  if (consecThread_.joinable()) {
    consecThread_.detach();
  }
}

void Sender::send(Protocol &protocol, Packet &&packet, Protocol::SendPacketCallback &&cb) {
  auto sender =
      std::make_shared<make_shared_enabler<Sender>>(protocol, std::move(packet), std::move(cb));
  sender->self_ = sender;
  return sender->send();
}

void Sender::finish(Error error) {
    Logger::debug("Calling callback");
  callback_(error);
  self_.reset();
}

void Sender::send() {
    std::shared_ptr<Sender> s = self_; // Hold the object for this scope
    try {
  if (packet_.size() <= 7) {
    std::vector<uint8_t> data = packet_.next(7);
    protocol_.sendSingleFrame(data);
    Logger::info("Sent single frame");
    finish(Error::Success);
  } else {
    waitForControlFrame();
    auto remaining = packet_.remaining();
    std::vector<uint8_t> data = packet_.next(6);
    protocol_.sendFirstFrame(remaining, data);
  }
   } catch (const std::exception &e) {
        Logger::critical("Failed to send ISO-TP message: " + std::string(e.what()));
    }
}

void Sender::waitForControlFrame() {
  timer_->setCallback([this] {
    connection_.reset();
    finish(Error::Timeout);
  });
  timer_->enable();

  connection_ = protocol_.listen([this](const Frame &f) {
    FlowControlFrame fc;
    if (!f.flow(fc)) {
      return;
    }
    timer_->disable();
    connection_.reset();
    onFlow(fc);
  });
}

uint8_t Sender::incrementConsec() {
  if (++consecIndex_ == 16) {
    consecIndex_ = 0;
  }
  return consecIndex_;
}

void Sender::onFlow(FlowControlFrame &frame) {
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
    finish(Error::Success);
      } catch (const std::exception &e) {
          Logger::critical("Error in isotp consecutive thread: " + std::string(e.what()));
          finish(Error::Unknown);
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
    setCan(can);
}

Protocol::~Protocol()
{
    Logger::debug("Destructed protocol");
}

void Protocol::setCan(std::unique_ptr<CanInterface> &&can) {
  can_ = can;
  if (!can) {
    canConnection_.reset();
    return;
  }

  canConnection_ =
      can_->connect(std::bind(&Protocol::onCan, this, std::placeholders::_1));
}

void Protocol::onCan(const CanMessage &message) {
  if (message.length() == 0 || message.id() != options().destId) {
    return;
  }

  signal_->call(Frame(message));
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
  return Sender::send(*this, std::move(packet), std::move(cb));
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
