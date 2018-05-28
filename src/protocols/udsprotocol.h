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

#ifndef UDSPROTOCOL_H
#define UDSPROTOCOL_H

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "isotpinterface.h"

/* Request SIDs */
#define UDS_REQ_SESSION 0x10
#define UDS_REQ_SECURITY 0x27
#define UDS_REQ_READMEM 0x23
#define UDS_REQ_REQUESTDOWNLOAD 0x34
#define UDS_REQ_REQUESTUPLOAD 0x35
#define UDS_REQ_TRANSFERDATA 0x36

/* Response SIDs (request + 0x40)*/
#define UDS_RES_SESSION 0x50
#define UDS_RES_SECURITY 0x67
#define UDS_RES_NEGATIVE 0x7F
#define UDS_RES_READMEM 0x63
#define UDS_RES_REQUESTUPLOAD 0x75
#define UDS_RES_REQUESTDOWNLOAD 0x74
#define UDS_RES_TRANSFERDATA 0x36

enum class UdsError {
  Success,
  Can,
  IsoTp,
  Timeout,
  BlankResponse,
  Unknown,
};

class UdsResponse {
public:
  UdsResponse(UdsError error) : error_(error) {};
  /* Successful response */
  UdsResponse(uint8_t id, const uint8_t *message, size_t length);
  /* Iso-Tp error'd response */
  UdsResponse(IsoTpError error);

  uint8_t id() const { return responseId_; }

  void setId(uint8_t id) { responseId_ = id; }

  size_t length() const { return message_.size(); }

  uint8_t &operator[](int index) { return message_[index]; }

  const uint8_t *message() const { return message_.data(); }

  const uint8_t &operator[](int index) const { return message_[index]; }

  void setMessage(const uint8_t *message, size_t length);

  bool success() const { return error_ == UdsError::Success; };
  
  UdsError error() const { return error_; };
  
  IsoTpError isotpError() const { return isotpError_; };
  
  std::string strError() const;
private:
  uint8_t responseId_ = 0;
  std::vector<uint8_t> message_;
  UdsError error_;
  IsoTpError isotpError_;
};

class UdsProtocol {
public:
  /* Create an interface with an ISO-TP layer */
  static std::shared_ptr<UdsProtocol> create(const std::shared_ptr<IsoTpInterface> &isotp,
                                             const IsoTpOptions &options);

  /* Sends a request. May throw an exception. */
  virtual boost::future<UdsResponse> request(const uint8_t *message, size_t length) = 0;

  /* All requests may throw an exception */
  /* Sends a DiagnosticSessionControl request */
  boost::future<UdsResponse> requestSession(uint8_t type);

  boost::future<UdsResponse> requestSecuritySeed();

  boost::future<UdsResponse> requestSecurityKey(const uint8_t *key, uint8_t length);

  /* ReadMemoryByAddress */
  boost::future<UdsResponse> requestReadMemoryAddress(uint32_t address, uint16_t length);

  virtual ~UdsProtocol(){};
};

#endif // UDSPROTOCOL_H
