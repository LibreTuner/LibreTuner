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
#include "datalogger.h"
#include "interface.h"
#include "j2534/j2534caninterface.h"
#include "protocols/isotpprotocol.h"
#include "protocols/socketcaninterface.h"
#include "protocols/udsprotocol.h"

void query_can(const std::shared_ptr<CanInterface> &can,
               DataLink::QueryVehicleCallback &&cb) {
  // Query the VIN
  std::array<uint8_t, 2> request{0x09, 0x02};
  uds::Protocol::create(std::make_shared<isotp::Protocol>(can))
      ->request(
          request, 0x49,
          [cb{std::move(cb)}](uds::Error error, const uds::Packet &packet) {
            if (error != uds::Error::Success) {
              if (error == uds::Error::Timeout) {
                cb(DataLink::Error::Timeout, nullptr);
                return;
              }
              cb(DataLink::Error::Protocol, nullptr);
              return;
            }

            if (packet.data.empty()) {
              cb(DataLink::Error::InvalidResponse, nullptr);
              return;
            }

            std::vector<uint8_t> data = packet.data;

            data.erase(data.begin());
            if (data.size() == 18) {
              // Mazda's weird shit?
              data.erase(data.begin());
            }

            if (data.size() != 17) {
              cb(DataLink::Error::InvalidResponse, nullptr);
              return;
            }

            std::string vin(reinterpret_cast<const char *>(data.data()),
                            data.size());
            cb(DataLink::Error::Success, Vehicle::fromVin(vin));
          });
}

#ifdef WITH_SOCKETCAN
class SocketCanDataLink : public DataLink {
public:
  explicit SocketCanDataLink(
      const std::shared_ptr<SocketCanSettings> &settings);

  void queryVehicle(QueryVehicleCallback &&cb) override;

  CanInterfacePtr can(uint32_t baudrate) override;

private:
  std::shared_ptr<SocketCanInterface> socketcan_;
};

SocketCanDataLink::SocketCanDataLink(
    const std::shared_ptr<SocketCanSettings> &settings) {
  protocols_ = DataLinkProtocol::Can;
  defaultProtocol_ = DataLinkProtocol::Can;
  socketcan_ = SocketCanInterface::create(settings->interface());
  socketcan_->start();
}

void SocketCanDataLink::queryVehicle(DataLink::QueryVehicleCallback &&cb) {
  query_can(socketcan_, std::move(cb));
}

CanInterfacePtr SocketCanDataLink::can(uint32_t baudrate) {
  return std::static_pointer_cast<CanInterface>(socketcan_);
}
#endif

#ifdef WITH_J2534
class J2534DataLink : public DataLink {
public:
  explicit J2534DataLink(const std::shared_ptr<J2534Settings> &settings);

  void queryVehicle(QueryVehicleCallback &&cb) override;

  CanInterfacePtr can(uint32_t baudrate) override;

  bool checkDevice();

private:
  j2534::J2534Ptr j2534_;
  std::shared_ptr<j2534::Can> can_;
  j2534::DevicePtr device_;
};

J2534DataLink::J2534DataLink(const std::shared_ptr<J2534Settings> &settings) {
  assert(settings->interface());
  j2534_ = settings->interface();
  if (!j2534_->initialized()) {
    j2534_->init();
  }
  protocols_ = j2534_->protocols();
  if ((protocols_ & DataLinkProtocol::Can) == DataLinkProtocol::Can) {
    defaultProtocol_ = DataLinkProtocol::Can;
  }
  if (!checkDevice()) {
      throw std::runtime_error("Failed to create j2534 device. Is one connected?");
  }
}

void J2534DataLink::queryVehicle(DataLink::QueryVehicleCallback &&cb) {
  // 500000 Hz is a good guess
  if (auto c = can(500000)) {
    query_can(c, std::move(cb));
  } else {
    cb(Error::NoConnection, nullptr);
  }
}

CanInterfacePtr J2534DataLink::can(uint32_t baudrate) {
  if (!can_ && checkDevice()) {
    can_ = j2534::Can::create(device_, baudrate);
  }
  return can_;
}

bool J2534DataLink::checkDevice() {
  assert(j2534_);
  if (device_) {
    return true;
  }
  device_ = j2534_->open();
  return !!device_;
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
#ifdef WITH_J2534
  case InterfaceType::J2534:
    return std::make_shared<J2534DataLink>(
        std::static_pointer_cast<J2534Settings>(iface));
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
