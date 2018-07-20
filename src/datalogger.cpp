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
#include "logger.h"

void DataLogger::setLog(const DataLogPtr &log) {
    log_ = log;
}

void DataLogger::addPid(uint32_t id, uint16_t code, const std::string &formula)
{
    addPid(Pid(id, code, formula));
}

UdsDataLogger::UdsDataLogger(const std::shared_ptr<uds::Protocol> &uds) : uds_(uds) {

}

void UdsDataLogger::addPid(Pid &&pid) {
  pids_.emplace_back(std::move(pid));
}

Pid *UdsDataLogger::nextPid() {
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
  Pid *pid = nextPid();
  if (!pid) {
    disable();
    return;
  }
  if (current_pid_ == 0) {
    //freeze();
    //return;
  }

  // Request the data
  std::vector<uint8_t> data;
  data.emplace_back(0x01);
  if (pid->code() > 0xFF) {
    data.emplace_back(pid->code() >> 8);
  }
  data.emplace_back(pid->code() & 0xFF);
  uds_->request(data, 0x41, [this, pid](uds::Error error, const uds::Packet &packet) {
    if (error != uds::Error::Success) {
      throwError("could not query data: " + uds::strError(error));
      return;
    }
    // Do calculations
    gsl::span<const uint8_t> data = gsl::make_span(packet.data);
    if (pid->code() > 0xFF) {
        data = data.subspan(2);
    } else {
        data = data.subspan(1);
    }

    switch (data.size()) {
    case 0:
        break;
    default:
    case 3:
        pid->setZ(data[2]);
    case 2:
        pid->setY(data[1]);
    case 1:
        pid->setX(data[0]);
        break;
    }

    if (log_) {
        double result = pid->evaluate();
        log_->add(pid->id(), result);
    }

    processNext();
  });
}

void UdsDataLogger::setErrorCallback(UdsDataLogger::ErrorCall &&error) {
  errorCall_ = std::move(error);
}

void UdsDataLogger::freeze() {
  /*std::array<uint8_t, 2> data{0x01, 0x02};
  uds_->request(data, 0x41, [this](uds::Error  error, const uds::Packet &packet) {
    if (error != uds::Error::Success) {
      throwError("could not freeze data: " + uds::strError(error));
      return;
    }
    current_pid_ = 0;
    processNext();
  });*/
  processNext();
}

void UdsDataLogger::throwError(const std::string &error) {
    Logger::critical("UdsDataLogger: " + error);
    if (errorCall_) {
        errorCall_(error);
    }
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


Pid::Pid(uint32_t id, uint16_t code, const std::string &formula) : id_(id), code_(code), formula_(formula)
{
    symbol_table_.add_variable("X", x_);
    symbol_table_.add_variable("Y", y_);
    symbol_table_.add_variable("Z", z_);

    expression_.register_symbol_table(symbol_table_);
    if (!parser_.compile(formula, expression_)) {
        throw std::runtime_error("expression could not compile.");
    }
}

Pid::Pid(Pid &&pid)
{
    id_ = pid.id_;
    formula_ = std::move(pid.formula_);
    code_ = pid.code_;

    symbol_table_.add_variable("X", x_);
    symbol_table_.add_variable("Y", y_);
    symbol_table_.add_variable("Z", z_);

    expression_.register_symbol_table(symbol_table_);
    if (!parser_.compile(formula_, expression_)) {
        throw std::runtime_error("expression could not compile.");
    }
}

double Pid::evaluate() const
{
    return expression_.value();
}
