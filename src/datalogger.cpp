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
#include "logger.h"

#include <utility>



void DataLogger::addPid(uint32_t id, uint16_t code,
                        const std::string &formula) {
    addPid(Pid(id, code, formula));
}



UdsDataLogger::UdsDataLogger(DataLog &log, std::unique_ptr<uds::Protocol> &&uds) : DataLogger(log), uds_(std::move(uds)) {}



void UdsDataLogger::addPid(Pid &&pid) { pids_.emplace_back(std::move(pid)); }



Pid *UdsDataLogger::nextPid() {
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
    Pid *pid = nextPid();
    if (!pid) {
        disable();
        return;
    }
    if (current_pid_ == 0) {
        // freeze();
        // return;
    }

    // Request the data
    std::vector<uint8_t> data;
    data.emplace_back(0x01);
    if (pid->code() > 0xFF) {
        data.emplace_back(pid->code() >> 8);
    }
    data.emplace_back(pid->code() & 0xFF);
    uds::Packet response;
    uds_->request(data.data(), data.size(), 0x41, response);
    
    
    if (pid->code() > 0xFF) {
        response.data.erase(response.data.begin(), response.data.begin() + 2);
    } else {
        response.data.erase(response.data.begin());
    }

    /*switch (data.size()) {
    case 0:
        break;
    default:
    case 3:
        pid->setZ(data[2]);
    case 2:
        pid->setY(data[1]);
    case 1:
        pid->setX(data[0]);
        break;
    }*/

    //double result = pid->evaluate();
    //log_.add(pid->id(), result);
}


void UdsDataLogger::run() {
    /*std::array<uint8_t, 2> data{0x01, 0x02};
    uds_->request(data, 0x41, [this](uds::Error  error, const uds::Packet
    &packet) { if (error != uds::Error::Success) { throwError("could not freeze
    data: " + uds::strError(error)); return;
      }
      current_pid_ = 0;
      processNext();
    });*/
    try {
        while (running_)
            processNext();
    } catch (const std::exception &e) {
        disable();
        Logger::warning(std::string("Error while running datalogger: ") + e.what() + ". Aborting.");
    }
}



void UdsDataLogger::disable() { running_ = false; }



Pid::Pid(uint32_t id, uint16_t code, const std::string &formula)
    : id_(id), code_(code) /*, expression_(formula.c_str()) */ {}




//double Pid::evaluate() const { return expression_.eval(vars_).asDouble(); }
