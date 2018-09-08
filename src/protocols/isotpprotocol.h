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

#ifndef LIBRETUNER_ISOTPPROTOCOL_H
#define LIBRETUNER_ISOTPPROTOCOL_H

#include "caninterface.h"
#include "timer.h"
#include <gsl/gsl>

// Std
#include <chrono>
#include <cstdint>

namespace isotp {
struct Options {
    unsigned sourceId = 0x7E0, destId = 0x7E8;
    std::chrono::milliseconds timeout{100};
};

class Packet {
public:
    Packet();
    Packet(Packet &&) = default;
    Packet &operator=(Packet &&packet);
    explicit Packet(gsl::span<const uint8_t> data);

    /* Sets the packet data and resets the pointer
     * to the beginning */
    void setData(gsl::span<const uint8_t> data);

    /* Returns the next data in the packet up to length max.
     * Increments the pointer */
    std::vector<uint8_t> next(size_t max);

    /* Returns the next byte and increments the pointer */
    uint8_t next();

    /* moves the data into the vector */
    void moveAll(std::vector<uint8_t> &data);

    /* Appends data to the packet. Does not use the pointer */
    void append(gsl::span<const uint8_t> data);

    std::vector<uint8_t>::size_type size() const { return data_.size(); }

    int remaining() const { return data_.end() - pointer_; }

    uint8_t &operator[](int index) { return data_[index]; }

    uint8_t operator[](int index) const { return data_[index]; }

    bool eof() const { return pointer_ == data_.end(); }

    void clear() {
        data_.clear();
        pointer_ = data_.begin();
    }

private:
    std::vector<uint8_t> data_;
    std::vector<uint8_t>::iterator pointer_ = data_.begin();
};

enum class FrameType {
    Single = 0,
    First = 1,
    Consecutive = 2,
    Flow = 3,
    None,
};

enum class FCFlag {
    Continue = 0,
    Wait = 1,
    Overflow = 2,
};

struct FlowControlFrame {
    FCFlag flag = FCFlag::Continue;
    uint8_t blockSize = 0;
    std::chrono::microseconds separationTime = std::chrono::microseconds(0);
};

struct SingleFrame {
    uint8_t size;
    std::array<uint8_t, 7> data;
};

struct FirstFrame {
    uint16_t size;
    std::array<uint8_t, 6> data;
    uint8_t data_length;
};

struct ConsecutiveFrame {
    uint8_t index;
    std::array<uint8_t, 7> data;
    uint8_t data_length;
};

struct Frame {
    std::array<uint8_t, 8> data{};
    size_t length = 0;

    Frame() = default;
    explicit Frame(const CanMessage &message);

    static Frame single(gsl::span<const uint8_t> data);
    static Frame first(uint16_t size, gsl::span<const uint8_t, 6> data);
    static Frame consecutive(uint8_t index, gsl::span<const uint8_t> data);
    static Frame flow(const FlowControlFrame &frame);

    FrameType type() const;

    /* Attempts to parse into a Single frame. Returns false if
     * the frame is malformed */
    bool single(SingleFrame &f) const;

    /* Attempts to parse into a First frame. Returns false if
     * the frame is malformed */
    bool first(FirstFrame &f) const;

    /* Attempts to parse into a Consecutive frame. Returns false if
     * the frame is malformed */
    bool consecutive(ConsecutiveFrame &f) const;

    /* Attempts to parse into a Flow Control frame. Returns false if
     * the frame is malformed */
    bool flow(FlowControlFrame &f) const;
};



class Protocol {
public:
    explicit Protocol(
        std::unique_ptr<CanInterface> &&can = std::unique_ptr<CanInterface>(),
        Options = Options());
    ~Protocol();

    void recv(Packet &result);

    // Sends a request and waits for a response
    void request(Packet &&req, Packet &result);

    void send(Packet &&packet);

    void setCan(std::unique_ptr<CanInterface> &&can);

    // May return nullptr
    CanInterface *can() { return can_.get(); }

    void setOptions(const Options &options) { options_ = options; }

    const Options &options() const { return options_; }

    /* Sends a frame to the CAN interface */
    void send(const Frame &frame);

private:
    std::unique_ptr<CanInterface> can_;
    Options options_;
    uint8_t consecIndex_;
    Packet packet_;

    // Sending
    void sendConsecutiveFrames();
    void recvFlowControlFrame(FlowControlFrame &frame);
    uint8_t incrementConsec();

    // Receiving
    void recvConsecutiveFrames(Packet &result, int remaining);

    bool recvFrame(Frame &frame);
};


void send_single_frame(Protocol &protocol, gsl::span<const uint8_t> data);

void send_first_frame(Protocol &protocol, uint16_t size,
                      gsl::span<const uint8_t, 6> data);

void send_consecutive_frame(Protocol &protocol, uint8_t index,
                            gsl::span<const uint8_t> data);

// Sends a flow control frame
void send_flow_frame(Protocol &protocol, const FlowControlFrame &frame);


namespace details {
uint8_t calculate_st(std::chrono::microseconds time);
std::chrono::microseconds calculate_time(uint8_t st);
FrameType frame_type(const CanMessage &message);
} // namespace details

} // namespace isotp

#endif // LIBRETUNER_ISOTPPROTOCOL_H
