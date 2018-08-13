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

#include <functional>
#include <gsl/span>
#include <memory>
#include <string>
#include <vector>

#include "isotpprotocol.h"

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
#define UDS_RES_TRANSFERDATA 0x76

/* Negative response codes */
// requestCorrectlyReceivedResponsePending
#define UDS_NRES_RCRRP 0x78

namespace uds {

struct Packet {
  uint8_t id;
  std::vector<uint8_t> data;
};

class Protocol {
public:
  /* Sends a request. May throw an exception. */
  virtual void request(gsl::span<uint8_t> data, uint8_t expectedId, Packet &response) = 0;

  /* All requests may throw an exception */
  /* Sends a DiagnosticSessionControl request. Returns parameter record. */
  std::vector<uint8_t> requestSession(uint8_t type);

  std::vector<uint8_t> requestSecuritySeed();

  void requestSecurityKey(gsl::span<uint8_t> key);

  /* ReadMemoryByAddress */
  std::vector<uint8_t> requestReadMemoryAddress(uint32_t address, uint16_t length);

  virtual ~Protocol() = default;
};



class IsoTpInterface : public Protocol {
public:
  explicit IsoTpInterface(std::unique_ptr<isotp::Protocol> &&isotp);

  void request(gsl::span<uint8_t> data, uint8_t expectedId, Packet &response) override;

private:
  std::unique_ptr<isotp::Protocol> isotp_;
};

} // namespace uds
#endif // UDSPROTOCOL_H
