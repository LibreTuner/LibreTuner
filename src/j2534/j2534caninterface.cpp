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

#include "j2534caninterface.h"
#include "logger.h"

#include <algorithm>

namespace j2534 {

std::shared_ptr<Can> Can::create(const DevicePtr &device, uint32_t baudrate)
{
    return std::make_shared<Can>(device, baudrate);
}

Can::Can(const DevicePtr &device, uint32_t baudrate) : channel_(device->connect(Protocol::CAN, 0, baudrate))
{

}

Can::~Can()
{
    Logger::debug("Destructing J2534 CAN interface");
    if (recvThread_.joinable()) {
        closed_ = true;
        recvThread_.join();
    }
}

void Can::send(const CanMessage &message)
{
    assert(valid());

    PASSTHRU_MSG msg;
    msg.ProtocolID = static_cast<uint32_t>(Protocol::CAN);
    msg.TxFlags = 0;

    // Add the CAN ID
    uint32_t id = message.id();
    msg.Data[0] = id & 0xFF000000;
    msg.Data[1] = id & 0xFF0000;
    msg.Data[2] = id & 0xFF00;
    msg.Data[3] = id & 0xFF;
    std::copy(message.message(), message.message() + message.length(), msg.Data + 4);
    // Message length + CAN ID length
    msg.DataSize = message.length() + 4;

    uint32_t numMsgs = 1;
    // TODO: Configurable timeout (100ms should be good for now, right?)
    channel_.writeMsgs(&msg, numMsgs, 100);
    if (numMsgs != 1) {
        throw std::runtime_error("Message write timed out");
    }
}

bool Can::valid()
{
    return channel_.valid();
}

void Can::start()
{
    assert(!recvThread_.joinable());
    closed_ = false;

    // TODO: Move this into j2534::J2534 to avoid creating many threads
    recvThread_ = std::thread([this]() {
        PASSTHRU_MSG msgs[16];
        uint32_t pNumMsgs;
        while (!closed_) {
            pNumMsgs = sizeof(msgs);
            // Give a timeout of 1000ms. In the future, this could be configured (TODO)
            try {
                channel_.readMsgs(msgs, pNumMsgs, 1000);
            } catch (const std::exception &ex) {
                Logger::critical("readMsgs error: " + std::string(ex.what()));
            }
            for (unsigned int i = 0; i < pNumMsgs; ++i) {
                // Read the CAN ID
                if (msgs[i].DataSize < 4) {
                    // The message does not fit the CAN ID
                    continue;
                }
                uint32_t id = (msgs[i].Data[0] << 24) | (msgs[i].Data[1] << 16) | (msgs[i].Data[2] << 8) | (msgs[i].Data[3]);
                signal_->call(CanMessage(id, gsl::make_span<uint8_t>(msgs[i].Data + 4, msgs[i].DataSize - 4)));
            }
        }
    });
}


}
