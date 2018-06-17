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

class SocketCanDataLink : public DataLink {
public:
  explicit SocketCanDataLink(
      const std::shared_ptr<SocketCanSettings> &settings);

  void queryVehicle(QueryVehicleCallback &&cb) override;

  CanInterfacePtr can() override;

private:
  std::shared_ptr<SocketCanInterface> socketcan_;
};

SocketCanDataLink::SocketCanDataLink(
    const std::shared_ptr<SocketCanSettings> &settings) {
  protocols_ = DataLinkProtocol::Can;
  defaultProtocol_ = DataLinkProtocol::Can;
  socketcan_ = SocketCanInterface::create(settings->interface());
}

void SocketCanDataLink::queryVehicle(DataLink::QueryVehicleCallback &&cb) {
  // Query the VIN
  std::array<uint8_t, 2> request{0x09, 0x02};
  uds::Protocol::create(std::make_shared<isotp::Protocol>(socketcan_))
      ->request(
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

            if (packet.data.size() != 17) {
              cb(Error::InvalidResponse, nullptr);
              return;
            }

            std::string vin(reinterpret_cast<const char *>(packet.data.data()),
                            packet.data.size());
            cb(Error::Success, Vehicle::fromVin(vin));
          });
}

CanInterfacePtr SocketCanDataLink::can() {
  return std::static_pointer_cast<CanInterface>(socketcan_);
}

DataLinkPtr DataLink::create(const InterfaceSettingsPtr &iface) {
  assert(iface);
  switch (iface->type()) {
  case InterfaceType::SocketCan:
    return std::make_shared<SocketCanDataLink>(
        std::static_pointer_cast<SocketCanSettings>(iface));
  default:
    throw std::runtime_error("Unsupported protocol");
  }
}
