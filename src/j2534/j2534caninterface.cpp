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
#include <sstream>

namespace j2534 {

std::shared_ptr<Can> Can::create(const DevicePtr &device, uint32_t baudrate)
{
    return std::make_shared<Can>(device, baudrate);
}

Can::Can(const DevicePtr &device, uint32_t baudrate) : channel_(device->connect(Protocol::CAN, CAN_ID_BOTH, baudrate))
{
    Logger::debug("Opened J2534 CAN Interface with baudrate " + std::to_string(baudrate));
    // Setup the filter
    PASSTHRU_MSG msgMask {};
    msgMask.ProtocolID = static_cast<uint32_t>(Protocol::CAN);
    msgMask.RxStatus = 0;
    msgMask.TxFlags = 0; //ISO15765_FRAME_PAD;
    msgMask.Timestamp = 0;
    msgMask.DataSize = 5;
    msgMask.ExtraDataIndex = 0;
    msgMask.Data[0] = 0;
    msgMask.Data[1] = 0;
    msgMask.Data[2] = 0;
    msgMask.Data[3] = 0;
    msgMask.Data[4] = 0;
    PASSTHRU_MSG msgPattern = msgMask;

    uint32_t msgId;
    channel_.startMsgFilter(PASS_FILTER, &msgMask, &msgPattern, nullptr, msgId);
}

Can::~Can()
{
    /*if (recvThread_.joinable()) {
        closed_ = true;
        recvThread_.join();
    }*/
}

void Can::send(const CanMessage &message)
{
    assert(valid());

    PASSTHRU_MSG msg;
    msg.ProtocolID = static_cast<uint32_t>(Protocol::CAN);
    msg.TxFlags = 0;

    // Add the CAN ID
    uint32_t id = message.id();
    msg.Data[0] = (id & 0xFF000000) >> 24;
    msg.Data[1] = (id & 0xFF0000) >> 16;
    msg.Data[2] = (id & 0xFF00) >> 8;
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

bool Can::recv(CanMessage &message, std::chrono::milliseconds timeout)
{
    assert(valid());

    auto start = std::chrono::system_clock::now();

    while (true) {
        PASSTHRU_MSG msg{};
        msg.ProtocolID = static_cast<uint32_t>(Protocol::CAN);

        uint32_t pNumMsgs = 1;
        channel_.readMsgs(&msg, pNumMsgs, timeout.count());
        if ((std::chrono::system_clock::now() - start) >= timeout) {
            return false;
        }

        if (msg.DataSize < 4) {
            // The message does not fit the CAN ID
            continue;
        }
        uint32_t id = (msg.Data[0] << 24) | (msg.Data[1] << 16) | (msg.Data[2] << 8) | (msg.Data[3]);
        message.setMessage(id, gsl::make_span<uint8_t>(msg.Data + 4, msg.DataSize - 4));
        return true;
    }
}

bool Can::valid()
{
    return channel_.valid();
}

void Can::start()
{
    /*
    assert(!recvThread_.joinable());
    closed_ = false;

    // TODO: Move this into j2534::J2534 to avoid creating many threads
    recvThread_ = std::thread([this]() {
        PASSTHRU_MSG msgs[16];
        uint32_t pNumMsgs;

        while (!closed_) {
            while (signal_->count() == 0) {
                // TODO: Use interrupt instead of polling
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            std::for_each(msgs, msgs + 16, [](PASSTHRU_MSG &msg) {
                msg.ProtocolID = static_cast<uint32_t>(Protocol::CAN);
            });
            pNumMsgs = 16;
            // Give a timeout of 1000ms. In the future, this could be configured (TODO)
            try {
                channel_.readMsgs(msgs, pNumMsgs, 1000);
            } catch (const std::exception &ex) {
                Logger::critical("readMsgs error: " + std::string(ex.what()));
            }
            for (unsigned int i = 0; i < pNumMsgs; ++i) {
                PASSTHRU_MSG &msg = msgs[i];
                // Read the CAN ID
                if (msgs[i].DataSize < 4) {
                    // The message does not fit the CAN ID
                    continue;
                }
                uint32_t id = (msg.Data[0] << 24) | (msg.Data[1] << 16) | (msg.Data[2] << 8) | (msg.Data[3]);
                CanMessage canMsg(id, gsl::make_span<uint8_t>(msgs[i].Data + 4, msgs[i].DataSize - 4));
                signal_->call(std::move(canMsg));
            }
        }
    });*/
}


}
