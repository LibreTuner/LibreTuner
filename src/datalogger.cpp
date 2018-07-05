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

#include "datalogger.h"

void DataLogger::setLog(const DataLogPtr &log) {
  log_ = log;
}

UdsDataLogger::UdsDataLogger(const std::shared_ptr<uds::Protocol> &uds) : uds_(uds) {

}

void UdsDataLogger::addPid(const PID &pid) {
  pids_.emplace_back(pid);
}

PID *UdsDataLogger::nextPid() {
  if (++current_pid_ >= pids_.size()) {
    current_pid_ = 0;
  }
  if (pids_.empty()) {
    return nullptr;
  }
  return &pids_[current_pid_];
}

void UdsDataLogger::processNext() {
  if (!running_) {
    return;
  }
  if (!uds_) {
    disable();
  }
  PID *pid = nextPid();
  if (!pid) {
    disable();
    return;
  }
  if (current_pid_ == 0) {
    freeze();
    return;
  }

  // Request the freeze data
  std::vector<uint8_t> data;
  data.emplace_back(0x02);
  if (pid->code > 0xFF) {
    data.emplace_back(pid->code >> 8);
  }
  data.emplace_back(pid->code & 0xFF);
  uds_->request(data, 0x42, [this](uds::Error error, const uds::Packet &packet) {
    if (error != uds::Error::Success) {
      throwError("could not query data: " + uds::strError(error));
      return;
    }
    // Do calculations
  });
}

void UdsDataLogger::setErrorCallback(UdsDataLogger::ErrorCall &&error) {
  errorCall_ = std::move(error);
}

void UdsDataLogger::freeze() {
  std::array<uint8_t, 2> data({0x01, 0x02});
  uds_->request(data, 0x41, [this](uds::Error  error, const uds::Packet &packet) {
    if (error != uds::Error::Success) {
      throwError("could not freeze data: " + uds::strError(error));
      return;
    }
    current_pid_ = 0;
    processNext();
  });
}

void UdsDataLogger::enable() {
  if (running_) {
    return;
  }
  if (uds_) {
    running_ = true;
    freeze();
  } else {
    throw std::runtime_error("a UDS device is not attached to the logger");
  }
}

void UdsDataLogger::disable() {
  running_ = false;
}

