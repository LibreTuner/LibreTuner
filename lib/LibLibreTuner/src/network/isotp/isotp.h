#ifndef ISOTP_H
#define ISOTP_H

#include "network/can/can.h"

#include <cstdint>
#include <chrono>
#include <vector>

namespace lt {
namespace network {

    
struct IsoTpOptions {
    uint32_t sourceId = 0x7E0, destId = 0x7E8;
    std::chrono::milliseconds timeout{10000};  
};



class IsoTpPacket {
public:
    IsoTpPacket();
    IsoTpPacket(const uint8_t *data, size_t size);

    /* Sets the packet data and resets the pointer
     * to the beginning */
    void setData(const uint8_t *data, size_t size);

    /* Returns the next data in the packet up to length max.
     * Increments the pointer */
    std::vector<uint8_t> next(size_t max);

    /* Returns the next byte and increments the pointer */
    uint8_t next();

    /* moves the data into the vector */
    void moveAll(std::vector<uint8_t> &data);

    /* Appends data to the packet. Does not use the pointer */
    void append(const uint8_t *data, size_t size);

    std::vector<uint8_t>::size_type size() const { return data_.size(); }

    uint16_t remaining() const { return data_.end() - pointer_; }

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



// ISO 15765-2 transport layer (ISO-TP) for sending large packets over CAN
class IsoTp {
public:
    // Takes ownership of a CAN interface
    IsoTp(CanPtr &&can = CanPtr(), IsoTpOptions = IsoTpOptions());
    ~IsoTp();

    void recv(IsoTpPacket &result);

    // Sends a request and waits for a response
    void request(IsoTpPacket &&req, IsoTpPacket &result);

    void send(IsoTpPacket &&packet);

    void setCan(CanPtr &&can);

    // May return nullptr
    Can *can() { return can_.get(); }

    void setOptions(const IsoTpOptions &options) { options_ = options; }

    const IsoTpOptions &options() const { return options_; }

    /* Sends a frame to the CAN interface */
    void send(const Frame &frame);

    void send_single_frame(const uint8_t *data, size_t size);

    void send_first_frame(uint16_t size, const uint8_t *data, size_t data_size);
                                                                                                                                                                                                                                           
    void send_consecutive_frame(uint8_t index,
                                const uint8_t *data, size_t size);

    // Sends a flow control frame
    void send_flow_frame(const FlowControlFrame &frame);

private:
    CanPtr can_;
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

} // namespace network
} // namespace lt

#endif // ISOTP_H
