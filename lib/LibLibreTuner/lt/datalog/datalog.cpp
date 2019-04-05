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

bool DataLog::add(const Pid &pid, PidLogEntry entry) {
    PidLog *log = pidLog(pid);
    if (log == nullptr) {
        return false;
    }

    if (empty_) {
        empty_ = false;
        beginTime_ = std::chrono::steady_clock::now();
    }

    log->entries.emplace_back(std::move(entry));
    return true;
}

PidLog *DataLog::pidLog(const Pid &pid) noexcept {
    auto it = logs_.find(pid.code);
    if (it == logs_.end()) {
        return nullptr;
    }
    return &it->second;
}

PidLog &DataLog::addPid(const Pid &pid) noexcept
{
    PidLog log{pid, {}};
    logs_.emplace(pid.code, std::move(log));
    return logs_.find(pid.code)->second;
}

bool DataLog::add(const Pid &pid, double value) {
    if (empty_) {
        empty_ = false;
        beginTime_ = std::chrono::steady_clock::now();
    }
    
    return add(pid, PidLogEntry{value, static_cast<std::size_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTime_).count())});
}

} // namespace lt
