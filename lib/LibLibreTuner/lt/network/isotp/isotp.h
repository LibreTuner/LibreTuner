#ifndef ISOTP_H
#define ISOTP_H

#include "../can/can.h"

#include <cstdint>
#include <chrono>
#include <vector>

namespace lt::network {

struct IsoTpOptions {
    uint32_t sourceId = 0x7E0, destId = 0x7E8;
    std::chrono::milliseconds timeout{1000};  
};

class IsoTpPacket {
public:
    IsoTpPacket();
    IsoTpPacket(const uint8_t *data, size_t size);

    /* Resets packet data to `data` */
    void setData(const uint8_t *data, size_t size);

    /* moves the data into the vector */
    void moveInto(std::vector<uint8_t> &data);

    /* Appends data to the end of the packet */
    void append(const uint8_t *data, size_t size);

    inline std::vector<uint8_t>::size_type size() const { return data_.size(); }

    inline uint8_t &operator[](int index) { return data_[index]; }

    inline uint8_t operator[](int index) const { return data_[index]; }

    inline void clear() {
        data_.clear();
    }

	inline std::vector<uint8_t>::iterator begin() { return data_.begin(); }
    inline std::vector<uint8_t>::const_iterator cbegin() const {
        return data_.cbegin(); }

    inline std::vector<uint8_t>::iterator end() { return data_.end(); }
    inline std::vector<uint8_t>::const_iterator cend() const { return data_.cend(); }

	inline uint8_t *data() { return data_.data(); }
    inline const uint8_t *data() const { return data_.data(); }

	inline bool empty() const { return data_.empty(); }

private:
    std::vector<uint8_t> data_;
};

class IsoTpPacketReader {
public:
	explicit IsoTpPacketReader(const IsoTpPacket &packet) : packet_(packet) {};

	inline std::size_t remaining() const { return packet_.size()  - pointer_; }

	// Returns the next bytes in the packet, stopping at `max` bytes
	std::vector<uint8_t> next(std::size_t max);
	// Returns the amount of bytes read
	std::size_t next(uint8_t *dest, std::size_t max);

	std::vector<uint8_t> readRemaining();

private:
	const IsoTpPacket &packet_;
	std::size_t pointer_{ 0 };
};

class IsoTp {
public:
    virtual void recv(IsoTpPacket &result) =0;

    // Sends a request and waits for a response
    virtual void request(const IsoTpPacket &req, IsoTpPacket &result) =0;

    virtual void send(const IsoTpPacket &packet) =0;

    virtual void setOptions(const IsoTpOptions &options) =0;
};
using IsoTpPtr = std::unique_ptr<IsoTp>;

} // namespace lt

#endif // ISOTP_H
