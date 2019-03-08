#ifndef CAN_H
#define CAN_H


#include <cstdint>
#include <array>
#include <chrono>
#include <cassert>
#include <memory>

namespace lt {
namespace network {

enum class CanError {
    Success = 0,
    Socket, // Socket creation error. err will be set
    Read,   // Read error. err will be set
    Write,
};



// Constants
constexpr std::size_t max_can_id = (1 << 30) - 1;



class CanMessage {
public:
    CanMessage() = default;
    CanMessage(uint32_t id, const uint8_t *message, std::size_t length);
    
    inline uint32_t id() const { return id_; }
    
    inline void setId(uint32_t id) {
        assert(id <= max_can_id);
        id_ = id;
    }
    
    inline const uint8_t *message() const { return message_.data(); }
    inline uint8_t *message() { return message_.data(); }
    
    inline uint8_t length() const { return length_; }
    
    inline void setMessage(std::array<uint8_t, 8> &&message, uint8_t length) { message_ = std::move(message); length_ = length; }
    void setMessage(const uint8_t *message, std::size_t length);
    
    inline void setMessage(uint32_t id, const uint8_t *message, std::size_t length) {
        setId(id);
        setMessage(message, length);
    }
private:
    std::array<uint8_t, 8> message_;
    uint8_t length_;
    uint32_t id_ = 0;
};



class Can {
public:
    Can();
    virtual ~Can() = default;

    // Send a CAN message. The size of data must be <= 8
    // Returns true if a message was sent
    void send(uint32_t id, const uint8_t *data, size_t length);

    virtual void send(const CanMessage &message) = 0;

    // Returns false if the timeout expired and no message was read
    virtual bool recv(CanMessage &message,
                      std::chrono::milliseconds timeout) = 0;
};

using CanPtr = std::unique_ptr<Can>;

} // namespace network
} // namespace lt

#endif // CAN_H
