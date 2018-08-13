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

struct Vehicle;
class VehicleLink;
using VehicleLinkPtr = std::shared_ptr<VehicleLink>;

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

  virtual ~DataLink();

  /* Creates a datalink from the interface settings. May throw an exception */
  static DataLinkPtr create(const InterfaceSettingsPtr &iface);

  // Attempts to query the vehicle
  virtual Vehicle queryVehicle() = 0;

  /* If CAN is supported, returns a CAN protocol. Else, returns
   * nullptr */
  virtual std::unique_ptr<CanInterface> can(uint32_t baudrate);

  /* Returns an ISO-TP protocol if supported. Else, returns
   * nullptr. By default, creates an ISO-TP interface from can() */
  virtual std::unique_ptr<isotp::Protocol> isotp();

protected:
  // Supported protocols
  DataLinkProtocol protocols_ = DataLinkProtocol::None;
  DataLinkProtocol defaultProtocol_ = DataLinkProtocol::None;
};

#endif // LIBRETUNER_DATALINK_H
