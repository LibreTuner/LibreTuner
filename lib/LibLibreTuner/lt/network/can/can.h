#ifndef CAN_H
#define CAN_H

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
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
    CanMessage(uint32_t id, const uint8_t *message, uint8_t length);

    inline uint32_t id() const noexcept { return id_; }

    inline void setId(uint32_t id) noexcept {
        assert(id <= max_can_id);
        id_ = id;
    }

    inline const uint8_t *message() const noexcept { return message_.data(); }
    inline uint8_t *message() noexcept { return message_.data(); }

    inline uint8_t &operator[](std::size_t index) { return message_[index]; }
    inline const uint8_t &operator[](std::size_t index) const {
        return message_[index];
    }

    inline void setLength(uint8_t length) noexcept {
        assert(length <= 8);
        length_ = length;
    }
    inline uint8_t length() const noexcept { return length_; }

    inline void setMessage(std::array<uint8_t, 8> &&message,
                           uint8_t length) noexcept {
        message_ = std::move(message);
        length_ = length;
    }
    void setMessage(const uint8_t *message, uint8_t length);

    inline void setMessage(uint32_t id, const uint8_t *message,
                           uint16_t length) {
        setId(id);
        setMessage(message, length);
    }

private:
    std::array<uint8_t, 8> message_{0};
    uint8_t length_;
    uint32_t id_ = 0;
};

class Can {
public:
    Can() = default;
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
