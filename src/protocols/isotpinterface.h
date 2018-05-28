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

#ifndef ISOTPINTERFACE_H
#define ISOTPINTERFACE_H

#include <chrono>
#include <memory>
#include <vector>
#include <functional>

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <boost/thread/future.hpp>

#include <QObject>

#include "caninterface.h"
#include "signal.h"

class IsoTpMessage {
public:
  IsoTpMessage(int id);

  // TODO: Remove this line \/ \/ \/ and add a [] operator
  const uint8_t *message() const { return message_.data(); }

  size_t length() const { return message_.size(); }

  int id() const { return id_; }

  /* Appends data to the message */
  void append(const uint8_t *message, size_t length);

private:
  std::vector<uint8_t> message_;
  int id_;
};



class IsoTpOptions {
public:
  IsoTpOptions(
      int srcId = 0x7DF, int dstId = 0x7E8,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(100));

  void setSrcId(int srcId) { srcId_ = srcId; }

  int srcId() const { return srcId_; }

  void setDstId(int dstId) { dstId_ = dstId; }

  int dstId() const { return dstId_; }

  void setTimeout(std::chrono::milliseconds timeout) { timeout_ = timeout; }

  std::chrono::milliseconds timeout() const { return timeout_; }

private:
  int srcId_, dstId_;
  std::chrono::milliseconds timeout_;
};

class IsoTpRequest;

enum class IsoTpError {
  Success,
  Unknown,
  Consec,
  Size,
};

class IsoTpInterface {
public:
  class Response {
  public:
    Response(std::unique_ptr<IsoTpMessage> message);
    Response(); // Constructed on timeout
    Response(IsoTpError error);
    Response(CanInterface::CanError error, int canErrno);

    /* Returns true if the request timed out */
    bool timedout() const { return error_ == Error::Timeout; };

    /* Returns true if the request was successful */
    bool success() const { return error_ == Error::Success; };

    enum class Error { Success, IsoTpError, CanError, Timeout };

    Error error() const { return error_; }

    std::string errorString() const;

    IsoTpMessage &message() const { return *message_; }
    
    IsoTpError isotpError() const { return isoTpError_; };
    
    CanInterface::CanError canError() const { return canError_; };
    
    int canErrno() const { return canErrno_; };

  private:
    std::unique_ptr<IsoTpMessage> message_;
    Error error_;
    union {
      IsoTpError isoTpError_;
      struct {
        CanInterface::CanError canError_;
        int canErrno_;
      };
    };
  };
  
  using Callback = std::function<void(const Response &response)>;

  /* Sends an isotp request. The response will be sent through
   * the onRecv() callback or onTimeout() will be called if
   * a response wasn't received in time. The timeout unit is milliseconds.
   * Returns false if a request is already being processed. */
  boost::future<Response> request(const uint8_t *message, size_t length, const IsoTpOptions &options);

  static std::string strError(IsoTpError error);

  /* Returns true if the interface is valid */
  bool valid() const { return can_->valid(); }

  std::shared_ptr<CanInterface> can() {
    return can_;
  }
  
  /* Returns the ISO-TP interface for the CAN device. Creates a new interface
   * if one does not exist. */
  static std::shared_ptr<IsoTpInterface> get(const std::shared_ptr<CanInterface> &can);
  
  ~IsoTpInterface();

private:
  
  IsoTpInterface(const std::shared_ptr<CanInterface> &can);

  std::shared_ptr<CanInterface> can_;
  std::vector<std::unique_ptr<IsoTpRequest>> requests_;
  
  static void removeDead();
};

#endif // ISOTPINTERFACE_H
