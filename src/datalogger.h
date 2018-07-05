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

#ifndef LIBRETUNER_DATALOGGER_H
#define LIBRETUNER_DATALOGGER_H

#include <cstdint>
#include <string>
#include <mutex>

#include "datalog.h"
#include "protocols/udsprotocol.h"

enum class PidType {
  Queried,
};

struct PID {
  DataLog::DataHead data;
  uint16_t code;
};

class DataLogger;
using DataLoggerPtr = std::shared_ptr<DataLogger>;

class DataLogger {
public:
  void setLog(const DataLogPtr &log);

  virtual ~DataLogger() = default;

  virtual void enable() =0;
  virtual void disable() =0;

protected:
  DataLogPtr log_;
};


class UdsDataLogger : public DataLogger {
public:
  explicit UdsDataLogger(const std::shared_ptr<uds::Protocol> &uds = std::shared_ptr<uds::Protocol>());
  UdsDataLogger(const UdsDataLogger&) = delete;
  UdsDataLogger(UdsDataLogger&&) = delete;

  using ErrorCall = std::function<void(const std::string &error)>;

  void addPid(const PID &pid);

  PID *nextPid();

  void enable() override;
  void disable() override;

  void processNext();

  void setErrorCallback(ErrorCall &&error);

private:
  void freeze();
  void throwError(const std::string &error);

  std::chrono::steady_clock::time_point freeze_time_;

  std::shared_ptr<uds::Protocol> uds_;
  std::vector<PID> pids_;

  std::mutex mutex_;
  bool running_ = false;
  size_t current_pid_ = 0;
  ErrorCall errorCall_;
};


#endif //LIBRETUNER_DATALOGGER_H
