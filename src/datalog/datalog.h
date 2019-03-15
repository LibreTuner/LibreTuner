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

#ifndef LIBRETUNER_DATALOG_H
#define LIBRETUNER_DATALOG_H

#include <chrono>
#include <unordered_map>
#include <vector>
#include <functional>

#include "util/signal.h"

/*
enum class DataUnit {
    None,
    Percentage,
    Degrees,
};

class DataLog {
public:
    explicit DataLog(definition::MainPtr platform);

    using TimePoint = std::chrono::steady_clock::time_point;
    struct DataHead {
        std::string name;
        std::string description;
        uint32_t id;
        DataUnit unit;
    };

    struct Data {
        definition::Pid &id;
        std::vector<std::pair<TimePoint, double>> values;

        explicit Data(definition::Pid &mid) : id(mid) {}
    };

    // Returns the time of the first data point
    TimePoint beginTime() const {
        return beginTime_;
    }
    
    using UpdateCall = std::function<void(const Data &info, double value, TimePoint time)>;

    // adds a point to a dataset. Returns false if the dataset
    // with the specified id does not exist. 
    bool add(uint32_t id, std::pair<TimePoint, double> value);

    // Adds a value at the current time
    bool add(uint32_t id, double value);

    // void addData(const DataHead &data);

    // Adds the pid with id `id` from the platform. Returns the added
    // id or nullptr if the pid does not exist.
    Data *addPid(uint32_t id);

    std::shared_ptr<Signal<UpdateCall>::ConnectionType> connectUpdate(UpdateCall &&call) { return updateSignal_->connect(std::move(call)) ;}
    
    std::string name() const { return name_; }
    void setName(const std::string &name) { name_ = name; }

    // Returns true if the log is empty
    bool empty() const { return empty_; }

private:
    TimePoint beginTime_;
    definition::MainPtr platform_;
    std::string name_;
    bool empty_{true};

    std::unordered_map<uint32_t, Data> data_;
    std::shared_ptr<Signal<UpdateCall>> updateSignal_;
};
using DataLogPtr = std::shared_ptr<DataLog>;
*/

#endif // LIBRETUNER_DATALOG_H
