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

#include "libretuner.h"
#include <array>
#include <iostream>
#include "j2534/j2534.h"
#include "j2534/j2534caninterface.h"
#include "protocols/isotpprotocol.h"
#include "logger.h"

int main(int argc, char *argv[]) {
  LibreTuner app(argc, argv);
/*
  j2534::Info info;
  info.functionLibrary = "C:\\Windows\\SysWOW64\\op20pt32.dll";
  info.protocols = DataLinkProtocol::Can;
  info.name = "OpenPort 2.0";

  j2534::J2534Ptr j2534 = j2534::J2534::create(std::move(info));
  j2534->init();

  j2534::DevicePtr device = j2534->open();
  std::shared_ptr<j2534::Can> can = j2534::Can::create(device, 500000);
  std::shared_ptr<isotp::Protocol> proto = std::make_shared<isotp::Protocol>(can, isotp::Options{0x7e0, 0x7e8, std::chrono::milliseconds{500}});
  std::array<uint8_t, 3> req = {0x1, 0x2, 0x3};
  proto->send(isotp::Packet(req), [](isotp::Error error) {
      Logger::debug("Sent the test request");
  });*/

  return LibreTuner::exec();
}
