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

#include "j2534.h"
#include "logger.h"

#include <cassert>
#include <sstream>

#include <windows.h>
namespace j2534 {

void J2534::init() { load(); }

DevicePtr J2534::open(char *port) {
  assert(initialized());

  uint32_t deviceId;
  long res;
  if ((res = PassThruOpen(port, &deviceId)) != 0) {
    if (res == 0x8) { // ERR_DEVICE_NOT_CONNECTED
      // Return nullptr. Don't throw an exception,
      // because the absence of a device is not an exceptional error
      return nullptr;
    }
    Logger::warning("PassThruOpen failed");
    throw std::runtime_error(lastError());
  }
  return std::make_shared<Device>(shared_from_this(), deviceId);
}

void J2534::close(uint32_t device) {
  assert(initialized());

  long res;
  if ((res = PassThruClose(device)) != 0) {
    throw std::runtime_error(lastError());
  }
}

uint32_t J2534::connect(uint32_t device, Protocol protocol, uint32_t flags,
                        uint32_t baudrate) {
  assert(initialized());

  long res;
  uint32_t channel;
  if ((res = PassThruConnect(device, static_cast<uint32_t>(protocol), flags,
                             baudrate, &channel)) != 0) {
    Logger::warning("PassThruConnect failed");
    throw std::runtime_error(lastError());
  }

  return channel;
}

void J2534::readMsgs(uint32_t channel, PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                     uint32_t timeout) {
  int32_t res = PassThruReadMsgs(channel, pMsg, &pNumMsgs, timeout);
  if (res != 0) {
    Logger::warning("PassThruReadMsgs failed");
    throw std::runtime_error(lastError());
  }
}

void J2534::writeMsgs(uint32_t channel, PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                      uint32_t timeout) {
  int32_t res = PassThruWriteMsgs(channel, pMsg, &pNumMsgs, timeout);
  if (res != 0) {
    Logger::warning("PassThruWriteMsgs failed");
    throw std::runtime_error(lastError());
  }
}

void J2534::disconnect(uint32_t channel) {
  assert(initialized());

  long res;
  if ((res = PassThruDisconnect(channel)) != 0) {
    throw std::runtime_error(lastError());
  }
}

std::string J2534::lastError() {
  char msg[80];
  PassThruGetLastError(msg);
  return std::string(msg);
}

J2534Ptr J2534::create(Info &&info) {
  return std::make_shared<J2534>(std::move(info));
}

J2534::~J2534() {
  if (hDll_) {
    CloseHandle(hDll_);
  }
}

void J2534::load() {
  if ((hDll_ = LoadLibrary(info_.functionLibrary.c_str())) == nullptr) {
    std::stringstream ss;
    ss << std::hex << GetLastError();
    throw std::runtime_error("Failed to load library " + info_.functionLibrary +
                             ": 0x" + ss.str());
  }

  PassThruOpen = reinterpret_cast<PassThruOpen_t>(getProc("PassThruOpen"));
  PassThruClose = reinterpret_cast<PassThruClose_t>(getProc("PassThruClose"));
  PassThruConnect =
      reinterpret_cast<PassThruConnect_t>(getProc("PassThruConnect"));
  PassThruDisconnect =
      reinterpret_cast<PassThruDisconnect_t>(getProc("PassThruDisconnect"));
  PassThruIoctl = reinterpret_cast<PassThruIoctl_t>(getProc("PassThruIoctl"));
  PassThruReadVersion =
      reinterpret_cast<PassThruReadVersion_t>(getProc("PassThruReadVersion"));
  PassThruGetLastError =
      reinterpret_cast<PassThruGetLastError_t>(getProc("PassThruGetLastError"));
  PassThruReadMsgs =
      reinterpret_cast<PassThruReadMsgs_t>(getProc("PassThruReadMsgs"));
  PassThruStartMsgFilter = reinterpret_cast<PassThruStartMsgFilter_t>(
      getProc("PassThruStartMsgFilter"));
  PassThruStopMsgFilter = reinterpret_cast<PassThruStopMsgFilter_t>(
      getProc("PassThruStopMsgFilter"));
  PassThruWriteMsgs =
      reinterpret_cast<PassThruWriteMsgs_t>(getProc("PassThruWriteMsgs"));
  PassThruStartPeriodicMsg = reinterpret_cast<PassThruStartPeriodicMsg_t>(
      getProc("PassThruStartPeriodicMsg"));
  PassThruStopPeriodicMsg = reinterpret_cast<PassThruStopPeriodicMsg_t>(
      getProc("PassThruStopPeriodicMsg"));
  PassThruSetProgrammingVoltage =
      reinterpret_cast<PassThruSetProgrammingVoltage_t>(
          getProc("PassThruSetProgrammingVoltage"));

  // If all exports were found (no exceptions were thrown), we can set loaded_
  // to true
  loaded_ = true;
}

void *J2534::getProc(const char *proc) {
  assert(hDll_);
  void *func = reinterpret_cast<void *>(
      GetProcAddress(reinterpret_cast<HMODULE>(hDll_), proc));
  if (!func) {
    throw std::runtime_error("Failed to get procedure from dll: " +
                             std::string(proc));
  }
  return func;
}

Device::Device(const J2534Ptr &j2534, uint32_t device)
    : j2534_(j2534), device_(device) {}

Device::~Device() {
    Logger::debug("Destructing J2534 device");
    close();
}

void Device::close() {
  if (valid()) {
    j2534_->close(device_);
    j2534_.reset();
  }
}

Channel Device::connect(Protocol protocol, uint32_t flags, uint32_t baudrate) {
  assert(valid());

  return Channel(j2534_, shared_from_this(),
                 j2534_->connect(device_, protocol, flags, baudrate));
}

Device::Device(Device &&dev) {
  device_ = dev.device_;
  j2534_ = std::move(dev.j2534_);
}

Channel::~Channel() {
  if (valid()) {
    j2534_->disconnect(channel_);
  }
}

Channel::Channel(Channel &&chann)
    : j2534_(std::move(chann.j2534_)), channel_(chann.channel_) {}

void Channel::readMsgs(PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                       uint32_t timeout) {
  assert(valid());
  j2534_->readMsgs(channel_, pMsg, pNumMsgs, timeout);
}

void Channel::writeMsgs(PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                        uint32_t timeout) {
  assert(valid());
  j2534_->writeMsgs(channel_, pMsg, pNumMsgs, timeout);
}

} // namespace j2534
