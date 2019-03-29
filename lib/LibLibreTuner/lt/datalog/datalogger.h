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

#ifndef LT_DATALOGGER_H
#define LT_DATALOGGER_H

#include <cstdint>
#include <mutex>
#include <string>
#include <atomic>

#include "datalog.h"
#include "../network/uds/uds.h"
#include <shunting-yard.h>

namespace lt {

enum class PidType {
    Queried,
};

class PidEvaluator {
public:
    PidEvaluator(const Pid &pid_);
    PidEvaluator(PidEvaluator &&) = default;
    PidEvaluator &operator=(PidEvaluator&&) = default;
    PidEvaluator &operator=(const PidEvaluator&) = delete;
    PidEvaluator(const PidEvaluator &) = delete;

    void setX(uint8_t x) { vars_["a"] = x; }
    void setY(uint8_t y) { vars_["b"] = y; }
    void setZ(uint8_t z) { vars_["c"] = z; }

    double evaluate() const;
    inline const Pid &pid() const noexcept { return pid_; }
    inline uint16_t code() const { return pid_.code; }

private:
    const Pid &pid_;

    TokenMap vars_;
    calculator expression_;
};

class DataLogger;

class DataLogger {
public:
    explicit DataLogger(DataLog &log) : log_(log) {}

    virtual ~DataLogger() = default;

    /* Sends a request to disable to logger. The logger will likely not be
     * disabled when this method returns */
    virtual void disable() = 0;

    /* Starts logging */
    virtual void run() = 0;

    virtual void addPid(Pid &&pid) = 0;

protected:
    DataLog &log_;
};
using DataLoggerPtr = std::unique_ptr<DataLogger>;


class UdsDataLogger : public DataLogger {
public:
    UdsDataLogger(DataLog &log, network::UdsPtr &&uds);
    UdsDataLogger(const UdsDataLogger &) = delete;
    UdsDataLogger(UdsDataLogger &&) = delete;
    UdsDataLogger &operator=(UdsDataLogger&&) = delete;
    UdsDataLogger &operator=(const UdsDataLogger&) = delete;
    
    ~UdsDataLogger() override = default;

    void addPid(Pid &&pid) override;

    PidEvaluator *nextPid();

    void processNext();

    void disable() override;

    /* Starts logging. */
    void run() override;

private:
    std::chrono::steady_clock::time_point freeze_time_;

    network::UdsPtr uds_;
    std::vector<PidEvaluator> pids_;

    std::atomic<bool> running_{false};
    size_t current_pid_ = 0;
};

}

#endif // LT_DATALOGGER_H
