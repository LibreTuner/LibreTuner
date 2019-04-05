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

#ifndef LT_DATALOG_H
#define LT_DATALOG_H

#include <chrono>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

#include "pid.h"
#include "../support/event.h"

namespace lt {
using DataLogTimePoint = std::chrono::steady_clock::time_point;

struct PidLogEntry {
    double value;
    // Miliseconds since log start
    std::size_t time;
};

struct PidLog {
    Pid pid;
    std::vector<PidLogEntry> entries;
};

class DataLog {
public:
    using AddEvent = Event<const PidLog &, const PidLogEntry &>;
    using AddConnectionPtr = AddEvent::ConnectionPtr;

    // Returns the time of the first data point
    DataLogTimePoint beginTime() const {
        return beginTime_;
    }
    
    // adds a point to a dataset. Returns false if the dataset
    // with the specified id does not exist. 
    bool add(const Pid &pid, PidLogEntry value);

    // Adds a value at the current time
    bool add(const Pid &pid, double value);
    
    // Returns the PID log or nullptr if it does not exist. Add with
    // addPid()
    PidLog *pidLog(const Pid &pid) noexcept;

    // Adds a PID to the log. Overwrites any previous logs with the same
    // pid.
    PidLog &addPid(const Pid &pid) noexcept;

    std::string name() const { return name_; }
    void setName(const std::string &name) { name_ = name; }

    // Returns true if the log is empty
    bool empty() const { return empty_; }
    
    template<typename Func>
    inline AddConnectionPtr onAdd(Func &&func) noexcept {
        return addEvent_.connect(std::forward<Func>(func));
    }

private:
    DataLogTimePoint beginTime_;
    std::string name_;
    bool empty_{true};
    
    AddEvent addEvent_;

    std::unordered_map<uint32_t, PidLog> logs_;
};
using DataLogPtr = std::shared_ptr<DataLog>;

}

#endif // LT_DATALOG_H
