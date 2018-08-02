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

#ifndef LIBRETUNER_DATALINK_H
#define LIBRETUNER_DATALINK_H

#include <functional>
#include <memory>
#include <string>

#include "vehicle.h"

class CanInterface;
using CanInterfacePtr = std::shared_ptr<CanInterface>;

class InterfaceSettings;
using InterfaceSettingsPtr = std::shared_ptr<InterfaceSettings>;

class DataLogger;
using DataLoggerPtr = std::shared_ptr<DataLogger>;

class DataLink;
using DataLinkPtr = std::shared_ptr<DataLink>;

namespace isotp {
class Protocol;
}

enum class DataLinkProtocol {
  None = 0,
  Can = 0x1,
};

inline DataLinkProtocol operator|(DataLinkProtocol lhs, DataLinkProtocol rhs) {
  using DType = std::underlying_type<DataLinkProtocol>::type;
  return static_cast<DataLinkProtocol>(static_cast<DType>(lhs) |
                                       static_cast<DType>(rhs));
}

inline DataLinkProtocol operator&(DataLinkProtocol lhs, DataLinkProtocol rhs) {
  using DType = std::underlying_type<DataLinkProtocol>::type;
  return static_cast<DataLinkProtocol>(static_cast<DType>(lhs) &
                                       static_cast<DType>(rhs));
}

/* Abstract DataLink interface (SocketCAN, J2534, ...) */
class DataLink {
public:
  enum class Error {
    Success,
    NoConnection,
    Protocol, // Protocol-level error
    NoProtocols,
    Timeout,
    InvalidResponse,
    Unknown,
  };

  virtual ~DataLink() = default;

  static std::string strError(Error error);

  /* Creates a datalink from the interface settings. May throw an exception */
  static DataLinkPtr create(const InterfaceSettingsPtr &iface);

  using QueryVehicleCallback = std::function<void(Error error, Vehicle &&)>;
  // Attempts to query the vehicle
  virtual void queryVehicle(QueryVehicleCallback &&cb) = 0;

  /* If CAN is supported, returns a CAN protocol. Else, returns
   * nullptr */
  virtual CanInterfacePtr can(uint32_t baudrate) { return nullptr; }

  /* Returns an ISO-TP protocol if supported. Else, returns
   * nullptr. By default, creates an ISO-TP interface from can() */
  virtual std::shared_ptr<isotp::Protocol> isotp() { return nullptr; }

protected:
  // Supported protocols
  DataLinkProtocol protocols_ = DataLinkProtocol::None;
  DataLinkProtocol defaultProtocol_ = DataLinkProtocol::None;
};

#endif // LIBRETUNER_DATALINK_H
