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
#include "logger.h"
#include "vehicle.h"

Vehicle query_can(std::unique_ptr<CanInterface> &&can) {
    // Query the VIN
    std::array<uint8_t, 2> request{0x09, 0x02};
    Logger::debug("Sending UDS query");
    std::unique_ptr<uds::Protocol> up = std::make_unique<uds::IsoTpInterface>(std::make_unique<isotp::Protocol>(std::move(can)));

    uds::Packet packet;
    up->request(request, 0x49, packet);

    if (packet.data.empty()) {
        throw std::runtime_error("empty uds response while querying VIN");
    }

    std::vector<uint8_t> data = std::move(packet.data);

    data.erase(data.begin());
    if (data.size() == 18) {
        // Mazda's weird shit?
        data.erase(data.begin());
    }

    if (data.size() != 17) {
        throw std::runtime_error("invalid VIN length");
    }

    std::string vin(reinterpret_cast<const char *>(data.data()),
                    data.size());
    return Vehicle::fromVin(vin);
}



#ifdef WITH_SOCKETCAN
class SocketCanDataLink : public DataLink {
public:
  explicit SocketCanDataLink(
      const std::shared_ptr<SocketCanSettings> &settings);

  Vehicle queryVehicle() override;

  std::unique_ptr<CanInterface> can(uint32_t baudrate) override;

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

Vehicle SocketCanDataLink::queryVehicle() {
  return query_can(socketcan_, std::move(cb));
}

CanInterfacePtr SocketCanDataLink::can(uint32_t baudrate) {
  return std::static_pointer_cast<CanInterface>(socketcan_);
}
#endif



#ifdef WITH_J2534
class J2534DataLink : public DataLink {
public:
  explicit J2534DataLink(const std::shared_ptr<J2534Settings> &settings);
  virtual ~J2534DataLink() override = default;

  Vehicle queryVehicle() override;

  std::unique_ptr<CanInterface> can(uint32_t baudrate) override;

  bool checkDevice();

private:
  j2534::J2534Ptr j2534_;
  //std::unique_ptr<j2534::Can> can_;
  j2534::DevicePtr device_;
};



J2534DataLink::J2534DataLink(const std::shared_ptr<J2534Settings> &settings) {
  Expects(settings->interface());
  j2534_ = settings->interface();
  if (!j2534_->initialized()) {
    j2534_->init();
  }
  protocols_ = j2534_->protocols();
  if ((protocols_ & DataLinkProtocol::Can) == DataLinkProtocol::Can) {
    defaultProtocol_ = DataLinkProtocol::Can;
  }
  if (!checkDevice()) {
      throw std::runtime_error("failed to create j2534 device. Is one connected?");
  }
}



Vehicle J2534DataLink::queryVehicle() {
  // 500000 Hz is a good guess
  if (auto c = can(500000)) {
    return query_can(std::move(c));
  }
  return Vehicle();
}



std::unique_ptr<CanInterface> J2534DataLink::can(uint32_t baudrate) {
  if (!checkDevice()) {
    return nullptr;
  }
  return std::make_unique<j2534::Can>(device_, baudrate);
}



bool J2534DataLink::checkDevice() {
  Expects(j2534_);
  if (device_) {
    return true;
  }
  device_ = j2534_->open();
  return !!device_;
}
#endif

DataLink::~DataLink()
{

}

DataLinkPtr DataLink::create(const InterfaceSettingsPtr &iface) {
  Expects(iface);
  switch (iface->type()) {
#ifdef WITH_SOCKETCAN
  case InterfaceType::SocketCan:
    return std::static_pointer_cast<DataLink>(std::make_shared<SocketCanDataLink>(
        std::static_pointer_cast<SocketCanSettings>(iface)));
#endif
#ifdef WITH_J2534
  case InterfaceType::J2534:
    return std::static_pointer_cast<DataLink>(std::make_shared<J2534DataLink>(
        std::static_pointer_cast<J2534Settings>(iface)));
#endif
  default:
    throw std::runtime_error("unsupported protocol");
  }
}

std::unique_ptr<CanInterface> DataLink::can(uint32_t baudrate)
{
    return nullptr;
}

std::unique_ptr<isotp::Protocol> DataLink::isotp()
{
    return nullptr;
}
