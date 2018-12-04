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

#include <sstream>

#include "isotpprotocol.h"
#include "logger.h"
#include "timer.h"
#include "util.hpp"

namespace isotp {
namespace details {

uint8_t calculate_st(std::chrono::microseconds time) {
    assert(time.count() >= 0);
    if (time.count() == 0) {
        return 0;
    }

    if (time >= std::chrono::milliseconds(1)) {
        return static_cast<uint8_t>(std::min<std::chrono::milliseconds::rep>(
            std::chrono::duration_cast<std::chrono::milliseconds>(time).count(),
            127));
    }
    uint8_t count = static_cast<uint8_t>(std::max<std::chrono::milliseconds::rep>(time.count() / 100, 1));
    return count + 0xF0;
}

std::chrono::microseconds calculate_time(uint8_t st) {
    if (st <= 127) {
        return std::chrono::milliseconds(st);
    }
    if (st >= 0xF1 && st <= 0xF9) {
        return std::chrono::microseconds((st - 0xF0) * 100);
    }
    return std::chrono::microseconds(0);
}

FrameType frame_type(const CanMessage &message) {
    if (message.length() == 0) {
        return FrameType::None;
    }

    uint8_t beg = message[0] & 0xF0;
    if (beg > 3) {
        return FrameType::None;
    }
    return static_cast<FrameType>(beg);
}

} // namespace details



Frame Frame::single(const uint8_t *data, size_t size) {
    assert(size <= 7);
    Frame f;
    f.data[0] = static_cast<uint8_t>(size);
    f.length = size + 1;
    std::copy(data, data + size, f.data.begin() + 1);
    return f;
}



Frame Frame::first(uint16_t size, const uint8_t *data, size_t data_size) {
    assert(size <= 6);
    Frame f;
    f.data[0] = 0x10 | ((size >> 8) & 0x0F);
    f.data[1] = static_cast<uint8_t>(size & 0xFF);
    f.length = data_size + 2;
    // Copy the data into the frame
    std::copy(data, data + data_size, f.data.begin() + 2);
    return f;
}



Frame Frame::consecutive(uint8_t index, const uint8_t *data, size_t size) {
    assert(size <= 7);
    assert(index <= 15);
    Frame f;
    // CAN-TP Header
    f.data[0] = 0x20 | index;
    f.length = size + 1;
    // Copy the data into the frame
    std::copy(data, data + size, f.data.begin() + 1);
    return f;
}



Frame Frame::flow(const FlowControlFrame &frame) {
    Frame f;
    f.data[0] = 0x30 | static_cast<uint8_t>(frame.flag);
    f.data[1] = frame.blockSize;
    f.data[2] = details::calculate_st(frame.separationTime);
    f.length = 3;
    return f;
}



bool Protocol::recvFrame(Frame &frame) {
    auto tStart = std::chrono::steady_clock::now();
    while (true) {
        CanMessage msg;
        if (!can_->recv(msg, options_.timeout)) {
            return false;
        }

        if (msg.length() == 0 || msg.id() != options_.destId) {
            if (std::chrono::steady_clock::now() - tStart >= options_.timeout) {
                // throw std::runtime_error("recvFrame timed out");
                return false;
            }
            continue;
        }
        frame = Frame(msg);
        return true;
    }
}



void Protocol::recvFlowControlFrame(FlowControlFrame &fc) {
    Frame frame;
    if (!recvFrame(frame)) {
        throw std::runtime_error("timed out");
    }

    if (!frame.flow(fc)) {
        // Unexpected
        throw std::runtime_error(
            "unexpected frame is not a flow control frame");
    }
}



uint8_t Protocol::incrementConsec() {
    if (++consecIndex_ == 16) {
        consecIndex_ = 0;
    }
    return consecIndex_;
}



void Protocol::sendConsecutiveFrames() {
    while (!packet_.eof()) {
        // Receive flow control
        FlowControlFrame fc;
        recvFlowControlFrame(fc);

        if (fc.flag == FCFlag::Overflow) {
            // Abort
            throw std::runtime_error("flow control frame requested abort");
        }

        if (fc.flag == FCFlag::Wait) {
            // Wait for another control frame
            continue;
        }

        consecIndex_ = 0;
        if (fc.separationTime.count() == 0) {
            // Avoid sending frames too quickly
            fc.separationTime = std::chrono::microseconds(100);
        }

        while (!packet_.eof()) {
            std::this_thread::sleep_for(fc.separationTime);
            std::vector<uint8_t> data = packet_.next(7);
            send_consecutive_frame(incrementConsec(), data.data(), data.size());

            if (fc.blockSize > 0) {
                if (fc.blockSize == 1) {
                    break;
                }
                fc.blockSize--;
            }
        }
    }
}



Frame::Frame(const CanMessage &message) {
    std::copy(message.message(), message.message() + message.length(),
              data.begin());
    length = message.length();
}



FrameType Frame::type() const {
    if (length == 0) {
        return FrameType::None;
    }

    uint8_t beg = (data[0] & 0xF0) >> 4;
    if (beg > 3) {
        return FrameType::None;
    }
    return static_cast<FrameType>(beg);
}



bool Frame::single(SingleFrame &f) const {
    if (type() != FrameType::Single) {
        return false;
    }

    uint8_t size = data[0] & 0x0F;
    if (length < size + 1) {
        return false;
    }

    // then this is a valid frame
    f.size = size;
    std::copy(data.begin() + 1, data.begin() + size + 1, f.data.begin());
    return true;
}



bool Frame::first(FirstFrame &f) const {
    if (type() != FrameType::First) {
        return false;
    }

    // Check if the frame is at least two bytes to fit the type and size header
    if (length < 2) {
        return false;
    }

    f.size = ((data[0] & 0x0F) << 8) | (data[1]);
    f.data_length = length - 2;
    std::copy(data.begin() + 2, data.begin() + length, f.data.begin());
    return true;
}



bool Frame::consecutive(ConsecutiveFrame &f) const {
    if (type() != FrameType::Consecutive) {
        return false;
    }

    f.index = data[0] & 0x0F;
    f.data_length = length - 1;
    std::copy(data.begin() + 1, data.begin() + length, f.data.begin());
    return true;
}



bool Frame::flow(FlowControlFrame &f) const {
    if (type() != FrameType::Flow) {
        return false;
    }

    if (length < 3) {
        return false;
    }

    uint8_t flag = data[0] & 0x0F;
    // Check that the flag is in the valid range (0, 1, 2)
    if (flag > 2) {
        return false;
    }

    f.flag = static_cast<FCFlag>(flag);
    f.blockSize = data[1];
    f.separationTime = details::calculate_time(data[2]);
    return true;
}



Packet::Packet() { pointer_ = std::begin(data_); }



Packet &Packet::operator=(Packet &&packet) {
    pointer_ = std::move(packet.pointer_);
    data_ = std::move(packet.data_);
    return *this;
}



Packet::Packet(const uint8_t *data, size_t size) { setData(data, size); }



void Packet::moveAll(std::vector<uint8_t> &data) {
    data = std::move(data_);
    pointer_ = std::begin(data_);
}



void Packet::append(const uint8_t *data, size_t size) {
    assert(data_.size() + size <= 4095);
    auto pOffset = std::distance(std::begin(data_), pointer_);
    data_.insert(data_.end(), data, data + size);
    pointer_ = std::begin(data_) + pOffset;
}



std::vector<uint8_t> Packet::next(size_t max) {
    size_t amount = std::min<size_t>(std::end(data_) - pointer_, max);
    auto begin = pointer_;
    pointer_ += amount;
    return std::vector<uint8_t>(begin, pointer_);
}



uint8_t Packet::next() { return *(pointer_++); }



void Packet::setData(const uint8_t *data, size_t size) {
    data_.assign(data, data + size);
    pointer_ = std::begin(data_);
}



Protocol::Protocol(std::unique_ptr<CanInterface> &&can, Options options)
    : options_(options) {
    setCan(std::move(can));
}



Protocol::~Protocol() = default;



void Protocol::setCan(std::unique_ptr<CanInterface> &&can) {
    can_ = std::move(can);
}



void Protocol::request(Packet &&req, Packet &result) {
    send(std::move(req));
    // Receive
    return recv(result);
}



void Protocol::recv(Packet &result) {
    // Wait for a single or first frame
    auto tStart = std::chrono::steady_clock::now();
    Frame frame;
    while (recvFrame(frame)) {
        if (frame.type() == FrameType::Single) {
            SingleFrame sf;
            if (!frame.single(sf)) {
                // Malformed
                throw std::runtime_error("malformed single frame");
            }
            result.setData(sf.data.data(), sf.size);
            // Received
            return;
        }
        if (frame.type() == FrameType::First) {
            FirstFrame ff;
            if (!frame.first(ff)) {
                throw std::runtime_error("malformed first frame");
            }
            result.append(ff.data.data(), ff.data_length);
            FlowControlFrame fc;
            send_flow_frame(fc);
            // Start receiving consec packets
            recvConsecutiveFrames(result, ff.size - ff.data_length);
            return;
        }
        if (std::chrono::steady_clock::now() - tStart >= options_.timeout) {
            throw std::runtime_error("recv timed out");
        }
    }

    // recvFrame timed out
    throw std::runtime_error("recvFrame timed out");
}



void Protocol::recvConsecutiveFrames(Packet &result, int remaining) {
    auto tStart = std::chrono::steady_clock::now();
    Frame frame;
    consecIndex_ = 0;
    while (recvFrame(frame)) {
        if (frame.type() == FrameType::Consecutive) {
            tStart = std::chrono::steady_clock::now();
            ConsecutiveFrame cf;
            if (!frame.consecutive(cf)) {
                throw std::runtime_error("malformed consecutive frame");
            }

            if (cf.index != incrementConsec()) {
                throw std::runtime_error("mismatched consecutive frame index");
            }

            int needed = std::min<int>(cf.data_length, remaining);

            result.append(cf.data.data(), needed);
            remaining -= needed;
            if (remaining <= 0) {
                // Finished
                return;
            }
        }

        if (std::chrono::steady_clock::now() - tStart >= options_.timeout) {
            throw std::runtime_error("recvConsecutiveFrames timed out");
        }
    }

    throw std::runtime_error("recvConsecutiveFrames timed out");
}



void Protocol::send(Packet &&packet) {
    if (packet.size() <= 7) {
        std::vector<uint8_t> data = packet.next(7);
        send_single_frame(data.data(), data.size());
    } else {
        uint16_t remaining = packet.remaining();
        std::vector<uint8_t> data = packet.next(6);
        send_first_frame(remaining, data.data(), data.size());

        packet_ = std::move(packet);
        // Start sending consecutive frames
        sendConsecutiveFrames();
    }
}



void Protocol::send(const Frame &frame) {
    assert(can_);
    can_->send(options_.sourceId, frame.data.data(), frame.data.size());
}



void Protocol::send_first_frame(uint16_t size,
                      const uint8_t *data, size_t data_size) {
    send(Frame::first(size, data, data_size));
}



void Protocol::send_consecutive_frame(uint8_t index,
                            const uint8_t *data, size_t size) {
    send(Frame::consecutive(index, data, size));
}



void Protocol::send_flow_frame(const isotp::FlowControlFrame &frame) {
    send(Frame::flow(frame));
}



void Protocol::send_single_frame(const uint8_t *data, size_t size) {
    send(Frame::single(data, size));
}

} // namespace isotp
