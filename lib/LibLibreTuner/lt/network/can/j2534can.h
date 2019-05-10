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

#ifndef J2534CANINTERFACE_H
#define J2534CANINTERFACE_H

#include <atomic>
#include <memory>
#include <thread>

#include "can.h"
#include "j2534/j2534.h"

namespace lt {
namespace network {

class J2534Can : public Can {
public:
    // Attempts to open a J2534 channel for CAN. May throw an exception
    J2534Can(const j2534::DevicePtr &device, uint32_t baudrate = 500000);

    virtual ~J2534Can() override;

    // Can interface
public:
    virtual void send(const CanMessage &message) override;
    // Returns true if a message was received before the timeout
    virtual bool recv(CanMessage &message,
                      std::chrono::milliseconds timeout) override;

private:
    j2534::Channel channel_;
};

} // namespace network
} // namespace lt
#endif // J2534CANINTERFACE_H
