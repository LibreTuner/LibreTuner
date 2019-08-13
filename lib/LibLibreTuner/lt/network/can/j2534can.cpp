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

#include "j2534can.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace lt
{
namespace network
{

J2534Can::J2534Can(const j2534::DevicePtr & device, uint32_t baudrate)
    : channel_(device->connect(j2534::Protocol::CAN, CAN_ID_BOTH, baudrate))
{
    // Setup the filter
    j2534::PASSTHRU_MSG msgMask{};
    msgMask.ProtocolID = static_cast<uint32_t>(j2534::Protocol::CAN);
    msgMask.RxStatus = 0;
    msgMask.TxFlags = 0; // ISO15765_FRAME_PAD;
    msgMask.Timestamp = 0;
    msgMask.DataSize = 5;
    msgMask.ExtraDataIndex = 0;
    msgMask.Data[0] = 0;
    msgMask.Data[1] = 0;
    msgMask.Data[2] = 0;
    msgMask.Data[3] = 0;
    msgMask.Data[4] = 0;
    j2534::PASSTHRU_MSG msgPattern = msgMask;

    uint32_t msgId;
    channel_.startMsgFilter(PASS_FILTER, &msgMask, &msgPattern, nullptr, msgId);
}

J2534Can::~J2534Can() = default;

void J2534Can::send(const CanMessage & message)
{
    j2534::PASSTHRU_MSG msg{};
    msg.ProtocolID = static_cast<uint32_t>(j2534::Protocol::CAN);
    msg.TxFlags = 0;

    // Add the CAN ID
    uint32_t id = message.id();
    msg.Data[0] = (id & 0xFF000000U) >> 24U;
    msg.Data[1] = (id & 0xFF0000U) >> 16U;
    msg.Data[2] = (id & 0xFF00U) >> 8U;
    msg.Data[3] = id & 0xFFU;
    std::copy(message.message(), message.message() + message.length(), msg.Data + 4);
    // Message length + CAN ID length
    msg.DataSize = message.length() + 4;

    uint32_t numMsgs = 1;
    // TODO: Configurable timeout (100ms should be good for now, right?)
    if (message.id() == 0x7e0)
    {
        std::ofstream file("j2534_out.txt", std::ios::app);
        file << "[" << std::hex << message.id() << "] ->";
        for (int i = 0; i < message.length(); ++i)
        {
            file << std::hex << " " << static_cast<uint32_t>(message[i]);
        }
        file << "\n";
        file.close();
    }
    channel_.writeMsgs(&msg, numMsgs, 100);
    if (numMsgs != 1)
    {
        throw std::runtime_error("Message write timed out");
    }
}

bool J2534Can::recv(CanMessage & message, std::chrono::milliseconds timeout)
{
    if (buffer_.pop(message))
        return true;

    auto start = std::chrono::system_clock::now();
    while (true)
    {
        if ((std::chrono::system_clock::now() - start) >= timeout)
            return false;

        std::array<j2534::PASSTHRU_MSG, 1> msgs{};
        for (auto & msg : msgs)
        {
            msg.ProtocolID = static_cast<uint32_t>(j2534::Protocol::CAN);
        }

        uint32_t pNumMsgs = msgs.size();
        channel_.readMsgs(msgs.data(), pNumMsgs, timeout.count());

        // Fill buffer
        for (int i = 0; i < pNumMsgs; ++i)
        {
            j2534::PASSTHRU_MSG & msg = msgs[i];
            if (msg.DataSize < 4)
            {
                // The message does not fit the CAN ID
                continue;
            }
            uint32_t id = (msg.Data[0] << 24U) | (msg.Data[1] << 16U) | (msg.Data[2] << 8U) | (msg.Data[3]);

            CanMessage can_msg;
            can_msg.setMessage(id, msg.Data + 4, msg.DataSize - 4);
            if (can_msg.id() == 0x7e8 || can_msg.id() == 0x7e0)
            {
                std::ofstream file("j2534_out.txt", std::ios::app);
                file << "[" << std::hex << can_msg.id() << "] <-";
                for (int i = 0; i < can_msg.length(); ++i)
                {
                    file << std::hex << " " << static_cast<uint32_t>(can_msg[i]);
                }
                file << "\n";
                file.close();
            }
            buffer_.add(can_msg);
        }
        if (buffer_.pop(message))
            return true;
    }
}

} // namespace network
} // namespace lt
