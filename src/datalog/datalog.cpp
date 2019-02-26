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
#include "logger.h"


bool DataLog::add(uint32_t id, std::pair<DataLog::TimePoint, double> value) {
    Data *data = addPid(id);
    if (data == nullptr) {
        return false;
    }

    if (empty_) {
        empty_ = false;
        beginTime_ = std::chrono::steady_clock::now();
    }

    data->values.emplace_back(value);
    updateSignal_->call(*data, value.second, value.first);
    return true;
}



bool DataLog::add(uint32_t id, double value) {
    return add(id, std::make_pair(std::chrono::steady_clock::now(), value));
}



DataLog::DataLog(definition::MainPtr platform) : platform_(std::move(platform)), updateSignal_(Signal<UpdateCall>::create()) {
    beginTime_ = std::chrono::steady_clock::now();
}



DataLog::Data *DataLog::addPid(uint32_t id) {
    auto it = data_.find(id);
    if (it != data_.end()) {
        return &it->second;
    }
    if (!platform_) {
        return nullptr;
    }

    definition::Pid *pid = platform_->getPid(id);
    if (pid != nullptr) {
        auto ret = data_.emplace(id, Data(*pid));
        return &ret.first->second;
    }
    return nullptr;
}
