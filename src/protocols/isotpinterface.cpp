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

#include "isotpinterface.h"
#include "timer.h"

#include <cassert>
#include <chrono>
#include <future>
#include <memory>
#include <exception>

#define BOOST_THREAD_PROVIDES_FUTURE
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

class IsoTpRequest {
public:
  IsoTpRequest(IsoTpInterface &interface, const IsoTpOptions options, const uint8_t *message, size_t length) : interface_(interface), options_(options), msg_(message, message + length) {
    assert(length <= 4095 && "Messsage is too long to to transmit over ISO-TP");
    connection_ = interface.can()->connect(std::bind(&IsoTpRequest::onCanMessage, this, std::placeholders::_1));
    // setTimeout(options_.timeout());
    send();
  }
  
  // void timedout() override;
  
  boost::future<IsoTpInterface::Response> future() {
    return promise_.get_future();
  }
  
private:
  enum class State {
    None,   // Waiting for a request
    Resp,   // Waiting for a single/first frame
    Consec, // Waiting for a consecutive frame
    SendingConsec,
    Flow, // Waiting for a flow control frame
  };

  boost::promise<IsoTpInterface::Response> promise_;
  
  State state_;
  
  void onCanMessage(const CanMessage &message);
  
  IsoTpInterface &interface_;
  IsoTpOptions options_;
  std::vector<uint8_t> msg_;
  std::vector<uint8_t>::iterator msgPtr_;
  
  std::unique_ptr<IsoTpMessage> recvMsg_;
  uint8_t consecIndex_;
  int recvRemaining_;
  std::thread consecThread_;
  
  std::shared_ptr<CanInterface::SignalType::ConnectionType> connection_;
  
  void send();
  
  void handleResponse(const CanMessage &message);
  void handleConsecutive(const CanMessage &message);
  void handleFlow(const CanMessage &message);
  
  /* Sends an ISO-TP First frame from msg_ */
  void sendFirst();
  
  /* Sends an ISO-TP Single frame from msg_ */
  void sendSingle();
  
  /* Sends an ISO-TP Flow Control frame */
  void sendFlowControl();
  
  void sendConsecFrames(uint8_t fcFlag, uint8_t blockSize, uint8_t sepTime);
  
  /* Called upon reception of the full ISO-TP response. Callc
   * the callback */
  void received();
  
  void throwError(IsoTpError error);
};
/*
void IsoTpRequest::timedout()
{
  state_ = State::None;
  recvMsg_.reset();
  connection_->disconnect();
  
  promise_.set_value(IsoTpInterface::Response());
}*/

void IsoTpRequest::throwError(IsoTpError error)
{
  state_ = State::None;
  recvMsg_.reset();
  connection_->disconnect();
  
  promise_.set_value(IsoTpInterface::Response(error));
}

void IsoTpRequest::sendSingle()
{
  // Sends the request in a single frame
  CanMessage msg;
  msg.id_ = options_.srcId();
  msg.messageLength_ = 8;
  
  unsigned length = msg_.size();
  assert(length < 8);
  
  // CAN-TP Header
  msg.message_[0] = length;
  
  std::copy(msg_.begin(), msg_.end(), &msg.message_[1]);
  
  recvMsg_ = std::make_unique<IsoTpMessage>(options_.dstId());
  state_ = State::Resp;
  // May throw an exception
  interface_.can()->send(msg);
}

void IsoTpRequest::sendFirst() {
  // Sends a first frame
  CanMessage msg;
  msg.id_ = options_.srcId();
  msg.messageLength_ = 8;
  
  unsigned length = msg_.size();
  // CAN-TP Header
  msg.message_[0] = 0x20 | ((length & 0xF00) >> 8);
  msg.message_[1] = length | 0xFF;
  
  std::copy(msg_.begin(), msg_.begin() + 6, &msg.message_[2]);
  
  msg_.erase(msg_.begin(), msg_.begin() + 6);
  msgPtr_ = msg_.begin();
  
  recvMsg_ = std::make_unique<IsoTpMessage>(options_.dstId());
  state_ = State::Flow;
  interface_.can()->send(msg);
}

void IsoTpRequest::send(){
  unsigned length = msg_.size();
  
  if (length < 8) {
    sendSingle();
  } else {
    sendFirst();
  }
  //startTimer();
}

void IsoTpRequest::sendFlowControl() {
  // Sends a flow control frame
  CanMessage resp;
  resp.id_ = options_.srcId();
  resp.messageLength_ = 8;
  resp[0] = 0x30; // FC flag = 0 (clear to send)
  resp[1] = 0;    // Block size
  resp[2] = 0;    // Time to delay between frames
  interface_.can()->send(resp);
}

void IsoTpRequest::received() {
  //stopTimer();
  state_ = State::None;
  promise_.set_value(IsoTpInterface::Response(std::move(recvMsg_)));
}

void IsoTpRequest::handleResponse(const CanMessage& message) {
  uint8_t type = message[0] & 0xF0;
  
  if (type == 0x0) {
    // Single frame
    if (message.length() < (message[0] & 0x0F) + 1) {
      // The CAN frame is not long enough
      throwError(IsoTpError::Size);
      return;
    }
    // Single frame
    recvMsg_->append(&message[1], message[0] & 0x0F);
    received();
    return;
  } else if (type == 0x10) {
    // First frame
    if (message.length() < 2) {
      // The CAN frame is not long enough
      throwError(IsoTpError::Size);
      return;
    }

    recvRemaining_ = ((message[0] & 0x0F) << 8) | message[1];
    if (recvRemaining_ < 7) {
      // The message should have been sent in a single frame
      if (message.length() < recvRemaining_ + 2) {
        // The CAN frame is not long enough
        throwError(IsoTpError::Size);
        return;
      }
      
      recvMsg_->append(&message[2], recvRemaining_);
      received();
      return;
    }
    
    if (message.length() < 8) {
      // The CAN frame is not long enough
      throwError(IsoTpError::Size);
      return;
    }
    
    recvMsg_->append(&message[2], 6);
    recvRemaining_ -= 6;
    sendFlowControl();
    consecIndex_ = 1;
    state_ = State::Consec;
  }
}

void IsoTpRequest::sendConsecFrames(uint8_t fcFlag, uint8_t blockSize, uint8_t sepTime) {
  uint8_t blocksRemaining = blockSize;
  
  CanMessage m;
  m.id_ = options_.srcId();
  m.messageLength_ = 8;
  
  std::chrono::microseconds duration;
  
  if (sepTime > 0) {
    if (sepTime <= 127) {
      duration = std::chrono::milliseconds(sepTime);
    } else if (sepTime >= 0xF1 && sepTime <= 0xF9) {
      duration = std::chrono::microseconds((sepTime - 0xF0) * 100);
    } else {
      duration = std::chrono::microseconds(0);
    }
  }
  
  while (msgPtr_ != msg_.end()) {
    if (duration.count() != 0) {
      std::this_thread::sleep_for(duration);
    }
    
    m[0] = 0x20 | consecIndex_;
    uint8_t toWrite = std::min<int>(7, msg_.end() - msgPtr_);
    std::copy(msgPtr_, msgPtr_ + toWrite, &m[1]);
    msgPtr_ += toWrite;
    
    interface_.can()->send(m);
    
    
    if (blocksRemaining > 0) {
      if (blocksRemaining == 1) {
        break;
      }
      
      --blocksRemaining;
    }
  }

  if (msgPtr_ == msg_.end()) {
    state_ = State::Resp;
  } else {
    state_ = State::Flow;
  }
}

void IsoTpRequest::handleConsecutive(const CanMessage& message) {
  uint8_t type = message[0] & 0xF0;
  
  if (type != 0x20) {
    return;
  }
  
  if ((message[0] & 0x0F) != consecIndex_) {
    // Incorrect consecutive consecutive index
    throwError(IsoTpError::Consec);
    return;
  }
  
  uint8_t toRead = std::min<int>(7, recvRemaining_);
  
  if (message.length() < toRead + 1) {
    // The CAN frame is not long enough
    throwError(IsoTpError::Size);
    return;
  }
  
  recvMsg_->append(&message[1], toRead);
  recvRemaining_ -= toRead;
  if (recvRemaining_ == 0) {
    received();
    return;
  }
  
  if (++consecIndex_ == 16) {
    consecIndex_ = 0;
  }
}

void IsoTpRequest::handleFlow(const CanMessage& message) {
  uint8_t type = message[0] & 0xF0;
  
  if (type != 0x30) {
    return;
  }
  
  
  if (message.length() < 3) {
    // The CAN frame is not long enough
    throwError(IsoTpError::Size);
    return;
  }
  
  uint8_t fcFlag = message[0] & 0x0F;
  uint8_t blockSize = message[1];
  uint8_t sepTime = message[2]; // Separation time
  consecIndex_ = 1;
  state_ = State::SendingConsec;
  consecThread_ = std::thread(std::bind(&IsoTpRequest::sendConsecFrames, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), fcFlag, blockSize, sepTime);
}

void IsoTpRequest::onCanMessage(const CanMessage& message) {
  if (message.id() != options_.dstId() || state_ == State::None) {
    return;
  }
  
  if (message.length() == 0) {
    return;
  }

  //stopTimer();
  
  switch (state_) {
    case State::Resp: 
      handleResponse(message);
      break;
    case State::Flow: 
      handleFlow(message);
      break;
    case State::Consec:
      handleConsecutive(message);
      
      break;
    default:
      break;
  }
  
  if (state_ != State::None && state_ != State::SendingConsec) {
    //startTimer();
  }
}

std::vector<std::pair<std::weak_ptr<CanInterface>, std::weak_ptr<IsoTpInterface>>> interfaces_;

void IsoTpInterface::removeDead()
{
  interfaces_.erase(std::remove_if(interfaces_.begin(), interfaces_.end(), [](const std::pair<std::weak_ptr<CanInterface>, std::weak_ptr<IsoTpInterface>> &i) -> bool {
    if (!i.first.lock() || !i.second.lock()) {
      return true;
    }
    return false;
  }), interfaces_.end());
}

std::shared_ptr<IsoTpInterface> IsoTpInterface::get(const std::shared_ptr<CanInterface> &can)
{
  removeDead();
  auto it = std::find_if(interfaces_.begin(), interfaces_.end(), [can](const std::pair<std::weak_ptr<CanInterface>, std::weak_ptr<IsoTpInterface>> &i) -> bool {
    if (std::shared_ptr<CanInterface> ptr = i.first.lock()) {
      if (ptr == can) {
        return true;
      }
    }
    return false;
  });
  
  if (it != interfaces_.end()) {
    if (std::shared_ptr<IsoTpInterface> ptr = it->second.lock()) {
      return ptr;
    }
  }
  
  std::shared_ptr<IsoTpInterface> isotp(new IsoTpInterface(can));
  interfaces_.push_back(std::make_pair(std::weak_ptr<CanInterface>(can), isotp));
  return isotp;
}

IsoTpMessage::IsoTpMessage(int id) : id_(id) {}

void IsoTpMessage::append(const uint8_t *message, size_t length) {
  message_.insert(message_.end(), message, message + length);
}

IsoTpOptions::IsoTpOptions(int srcId,
                           int dstId, std::chrono::milliseconds timeout)
    : srcId_(srcId), dstId_(dstId), timeout_(timeout) {}

IsoTpInterface::IsoTpInterface(const std::shared_ptr<CanInterface> &can)
    : can_(can) {
      
}

boost::future<IsoTpInterface::Response> IsoTpInterface::request(const uint8_t* message, size_t length, const IsoTpOptions &options) {
  std::unique_ptr<IsoTpRequest> request = std::make_unique<IsoTpRequest>(*this, options, message, length);
  boost::future<IsoTpInterface::Response> future = request->future();
  requests_.push_back(std::move(request));
  return future;
}

std::string IsoTpInterface::Response::errorString() const
{
  switch(error_) {
    case Error::Success:
      return "success";
    case Error::CanError:
      return "CAN error";
    case Error::IsoTpError:
      return "ISO-TP Error";
    case Error::Timeout:
      return "timed out";
  }
  return "unknown error";
}

IsoTpInterface::Response::Response() : error_(Error::Timeout)
{
}

IsoTpInterface::Response::Response(CanInterface::CanError error, int canErrno) : error_(Error::CanError), canError_(error), canErrno_(canErrno)
{
}

IsoTpInterface::Response::Response(IsoTpError error) : error_(Error::IsoTpError), isoTpError_(error)
{
}

IsoTpInterface::Response::Response(std::unique_ptr<IsoTpMessage> message) : message_(std::move(message)), error_(Error::Success)
{
}

std::string IsoTpInterface::strError(IsoTpError error) {
  switch (error) {
  case IsoTpError::Consec:
    return "a received consecutive frame index was incorrect";
  case IsoTpError::Size:
    return "a received CAN frame was too short";
  case IsoTpError::Success:
    return "success";
  case IsoTpError::Unknown:
    return "unknown";
  default:
    break;
  }

  return "unknown. You should never see this. If you do, please submit a pull "
         "request";
}

IsoTpInterface::~IsoTpInterface() {

}
