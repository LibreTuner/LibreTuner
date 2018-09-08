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
#include <mutex>
#include <string>
#include <thread>

#include "datalog.h"
#include "protocols/udsprotocol.h"
#include <shunting-yard.h>

enum class PidType {
    Queried,
};

class Pid {
public:
    Pid(uint32_t id, uint16_t code, const std::string &formula);
    Pid(Pid &&);
    Pid(const Pid &) = delete;

    void setX(uint8_t x) { vars_["a"] = x; }
    void setY(uint8_t y) { vars_["b"] = y; }
    void setZ(uint8_t z) { vars_["c"] = z; }

    double evaluate() const;
    uint32_t id() const { return id_; }

    uint16_t code() const { return code_; }

private:
    uint32_t id_;
    uint16_t code_;

    TokenMap vars_;
    calculator expression_;
};

class DataLogger;

class DataLogger {
public:
    void setLog(const DataLogPtr &log);

    virtual ~DataLogger() = default;

    virtual void enable() = 0;
    virtual void disable() = 0;
    /* Returns true if the logger is running */
    virtual bool running() const = 0;

    virtual void addPid(Pid &&pid) = 0;

    virtual void addPid(uint32_t id, uint16_t code, const std::string &formula);

protected:
    DataLogPtr log_;
};


class UdsDataLogger : public DataLogger {
public:
    explicit UdsDataLogger(std::unique_ptr<uds::Protocol> &&uds);
    UdsDataLogger(const UdsDataLogger &) = delete;
    UdsDataLogger(UdsDataLogger &&) = delete;

    using ErrorCall = std::function<void(const std::string &error)>;

    void addPid(Pid &&pid) override;

    Pid *nextPid();

    void enable() override;
    void disable() override;
    bool running() const override { return running_; }

    void processNext();

    void setErrorCallback(ErrorCall &&error);

private:
    /* Thread routine */
    void run();
    
    void throwError(const std::string &error);

    std::chrono::steady_clock::time_point freeze_time_;

    std::unique_ptr<uds::Protocol> uds_;
    std::vector<Pid> pids_;

    std::mutex mutex_;
    bool running_ = false;
    size_t current_pid_ = 0;
    ErrorCall errorCall_;
    
    std::thread thread_;
};


#endif // LIBRETUNER_DATALOGGER_H
