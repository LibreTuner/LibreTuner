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

#include "datalog.h"

namespace lt {
    
void DataLog::add(const Pid &pid, std::pair<DataLogTimePoint, double> value) {
    PidLog *log = pidLog(pid);
    if (log == nullptr) {
        // Create log
        data_.emplace(pid.id, PidLog(pid));
        log = pidLog(pid);
    }

    if (empty_) {
        empty_ = false;
        beginTime_ = std::chrono::steady_clock::now();
    }

    log->add(value);
}

PidLog *DataLog::pidLog(const Pid &pid) noexcept {
    auto it = data_.find(pid.id);
    if (it == data_.end()) {
        return nullptr;
    }
    return &it->second;
}

void DataLog::add(const Pid &pid, double value) {
    add(pid, std::make_pair(std::chrono::steady_clock::now(), value));
}



DataLog::DataLog(PlatformPtr platform) : platform_(std::move(platform)) {
    beginTime_ = std::chrono::steady_clock::now();
}

} // namespace lt
