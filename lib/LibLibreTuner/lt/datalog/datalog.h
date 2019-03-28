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

#include "../definition/platform.h"

namespace lt {

using DataLogTimePoint = std::chrono::steady_clock::time_point;

struct PidLog {
    explicit PidLog(const Pid &_pid) : pid(_pid) {}
    
    void add(std::pair<DataLogTimePoint, double> value) { values.emplace_back(value); }
    
    const Pid &pid;
    std::vector<std::pair<DataLogTimePoint, double>> values;
};

class DataLog {
public:
    explicit DataLog(PlatformPtr platform);
    
    // Returns the time of the first data point
    DataLogTimePoint beginTime() const {
        return beginTime_;
    }
    
    // adds a point to a dataset. Returns false if the dataset
    // with the specified id does not exist. 
    void add(const Pid &pid, std::pair<DataLogTimePoint, double> value);

    // Adds a value at the current time
    void add(const Pid &pid, double value);
    
    PidLog *pidLog(const Pid &pid) noexcept;

    std::string name() const { return name_; }
    void setName(const std::string &name) { name_ = name; }

    // Returns true if the log is empty
    bool empty() const { return empty_; }

private:
    DataLogTimePoint beginTime_;
    PlatformPtr platform_;
    std::string name_;
    bool empty_{true};

    std::unordered_map<uint32_t, PidLog> data_;
};
using DataLogPtr = std::shared_ptr<DataLog>;

}

#endif // LT_DATALOG_H
