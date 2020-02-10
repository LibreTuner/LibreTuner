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

#include <utility>

// #include <shunting-yard.h>

// extern void cparse_startup();

namespace lt
{

class PidEvaluator
{
public:
    explicit PidEvaluator(const Pid & pid)
        : pid_(pid) /*, expression_(pid.formula.c_str())*/
    {
    }
    PidEvaluator(PidEvaluator &&) = default;
    PidEvaluator & operator=(const PidEvaluator &) = delete;
    PidEvaluator(const PidEvaluator &) = delete;

    void setX(uint8_t x) { /*vars_["a"] = x;*/ }
    void setY(uint8_t y) { /*vars_["b"] = y;*/ }
    void setZ(uint8_t z) { /*vars_["c"] = z;*/ }

    double evaluate() const { return 0; /*expression_.eval(vars_).asDouble();*/ }

    inline const Pid & pid() const noexcept { return pid_; }
    inline uint16_t code() const { return pid_.code; }

private:
    const Pid & pid_;

    // TokenMap vars_;
    // calculator expression_;
};

class CParseInit
{
public:
    CParseInit() { /*cparse_startup();*/ }
};

UdsDataLogger::UdsDataLogger(DataLog & log, network::UdsPtr && uds)
    : DataLogger(log), uds_(std::move(uds)), iter_(pids_.begin())
{
    static CParseInit cparseInit;
}

void UdsDataLogger::addPid(Pid pid) { pids_.emplace_front(std::move(pid)); }

Pid * UdsDataLogger::nextPid()
{
    if (pids_.empty())
    {
        return nullptr;
    }

    if (iter_ == pids_.end())
    {
        iter_ = pids_.begin();
    }

    return &*iter_++;
}

void UdsDataLogger::processNext()
{
    if (!uds_)
    {
        disable();
    }
    Pid * pid = nextPid();
    if (pid == nullptr)
    {
        // PID list is empty. Disable to avoid infinite loop
        disable();
        return;
    }
    if (current_pid_ == 0)
    {
        // Freeze current frame
        // freeze();
        // return;
    }

    // Request the data
    std::vector<uint8_t> response = uds_->readDataByIdentifier(pid->code);

    PidEvaluator evaluator(*pid);

    switch (response.size())
    {
    case 0:
        break;
    default:
    case 3:
        evaluator.setZ(response[2]);
        // Fallthrough
    case 2:
        evaluator.setY(response[1]);
        // Fallthrough
    case 1:
        evaluator.setX(response[0]);
        break;
    }

    double result = evaluator.evaluate();
    log_.add(*pid, result);
}

void UdsDataLogger::run()
{
    running_ = true;
    try
    {
        while (running_)
            processNext();
    }
    catch (const std::exception & e)
    {
        disable();
    }
}

void UdsDataLogger::disable() { running_ = false; }

} // namespace lt
