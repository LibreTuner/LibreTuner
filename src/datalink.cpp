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

#include "datalink.h"
#include "protocols/isotpprotocol.h"
#include "protocols/socketcaninterface.h"
#include "protocols/udsprotocol.h"
#include "datalogger.h"
#include "interface.h"

#ifdef WITH_SOCKETCAN
class SocketCanDataLink : public DataLink {
public:
  explicit SocketCanDataLink(
      const std::shared_ptr<SocketCanSettings> &settings);

  void queryVehicle(QueryVehicleCallback &&cb) override;

  CanInterfacePtr can() override;

private:
  std::shared_ptr<SocketCanInterface> socketcan_;
  std::shared_ptr<uds::Protocol> uds_;
};

SocketCanDataLink::SocketCanDataLink(
    const std::shared_ptr<SocketCanSettings> &settings) {
  protocols_ = DataLinkProtocol::Can;
  defaultProtocol_ = DataLinkProtocol::Can;
  socketcan_ = SocketCanInterface::create(settings->interface());
  socketcan_->start();
}

void SocketCanDataLink::queryVehicle(DataLink::QueryVehicleCallback &&cb) {
  // Query the VIN
  std::array<uint8_t, 2> request{0x09, 0x02};
  uds_ = uds::Protocol::create(std::make_shared<isotp::Protocol>(socketcan_));
  uds_->request(
          request, 0x49,
          [cb{std::move(cb)}](uds::Error error, const uds::Packet &packet) {
            if (error != uds::Error::Success) {
              if (error == uds::Error::Timeout) {
                cb(Error::Timeout, nullptr);
                return;
              }
              cb(Error::Protocol, nullptr);
              return;
            }

            if (packet.data.empty()) {
              cb(Error::InvalidResponse, nullptr);
              return;
            }

            std::vector<uint8_t> data = packet.data;

            data.erase(data.begin());
            if (data.size() == 18) {
              // Mazda's weird shit
              data.erase(data.begin());
            }

            if (data.size() != 17) {
              cb(Error::InvalidResponse, nullptr);
              return;
            }

            std::string vin(reinterpret_cast<const char *>(data.data()),
                            data.size());
            cb(Error::Success, Vehicle::fromVin(vin));
          });
}

CanInterfacePtr SocketCanDataLink::can() {
    return std::static_pointer_cast<CanInterface>(socketcan_);
}
#endif

DataLinkPtr DataLink::create(const InterfaceSettingsPtr &iface) {
  assert(iface);
  switch (iface->type()) {
#ifdef WITH_SOCKETCAN
  case InterfaceType::SocketCan:
    return std::make_shared<SocketCanDataLink>(
        std::static_pointer_cast<SocketCanSettings>(iface));
#endif
  default:
    throw std::runtime_error("Unsupported protocol");
  }
}

std::string DataLink::strError(DataLink::Error error) {
  switch (error) {
    case Error::Success:
      return "success";
    case Error::Protocol:
      return "protocol-level error";
    case Error::InvalidResponse:
      return "invalid response";
    case Error::Timeout:
      return "timed out. Is the device connected?";
    case Error::Unknown:
      return "unknown";
    case Error::NoConnection:
      return "no connection";
    case Error::NoProtocols:
      return "no usable protocols";
    default:
      return "unknown";
  }
}
