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

extern void cparse_startup();

namespace lt {


class CParseInit {
public:
    CParseInit() {
        cparse_startup();
    }
};


UdsDataLogger::UdsDataLogger(DataLog &log, network::UdsPtr &&uds) : DataLogger(log), uds_(std::move(uds)) {
    static CParseInit cparseInit;
}



void UdsDataLogger::addPid(Pid &&pid) { pids_.emplace_back(std::move(pid)); }



PidEvaluator *UdsDataLogger::nextPid() {
    if (++current_pid_ >= pids_.size()) {
        current_pid_ = 0;
    }
    if (pids_.empty()) {
        return nullptr;
    }
    return &pids_[current_pid_];
}



void UdsDataLogger::processNext() {
    if (!uds_) {
        disable();
    }
    PidEvaluator *pid = nextPid();
    if (!pid) {
        // PID list is empty. Disable to avoid infinite loop
        disable();
        return;
    }
    if (current_pid_ == 0) {
        // Freeze current frame
        // freeze();
        // return;
    }

    // Request the data
    std::vector<uint8_t> response = uds_->readDataByIdentifier(pid->code());

    switch (response.size()) {
    case 0:
        break;
    default:
    case 3:
        pid->setZ(response[2]);
    case 2:
        pid->setY(response[1]);
    case 1:
        pid->setX(response[0]);
        break;
    }

    double result = pid->evaluate();
    log_.add(pid->pid(), result);
}


void UdsDataLogger::run() {
    running_ = true;
    try {
        while (running_)
            processNext();
    } catch (const std::exception &e) {
        disable();
    }
}



void UdsDataLogger::disable() { running_ = false; }



PidEvaluator::PidEvaluator(const Pid &pid)
    : pid_(pid), expression_(pid.formula.c_str()) {}




double PidEvaluator::evaluate() const { return expression_.eval(vars_).asDouble(); }

}
