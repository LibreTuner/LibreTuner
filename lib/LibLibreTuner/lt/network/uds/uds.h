#ifndef LT_UDS_H
#define LT_UDS_H

#include <cstdint>
#include <memory>
#include <vector>

namespace lt {
namespace network {

/* Request SIDs */
constexpr uint8_t UDS_REQ_SESSION = 0x10;
constexpr uint8_t UDS_REQ_SECURITY = 0x27;
constexpr uint8_t UDS_REQ_READMEM = 0x23;
constexpr uint8_t UDS_REQ_REQUESTDOWNLOAD = 0x34;
constexpr uint8_t UDS_REQ_REQUESTUPLOAD = 0x35;
constexpr uint8_t UDS_REQ_TRANSFERDATA = 0x36;

constexpr uint8_t UDS_RES_NEGATIVE = 0x7F;

/* Negative response codes */
// requestCorrectlyReceivedResponsePending
constexpr uint8_t UDS_NRES_RCRRP = 0x78;

struct UdsResponse {
    std::vector<uint8_t> data;
    
    bool negative{false};
    uint8_t negativeCode{0};
};

class Uds {
  public:
    /* Sends a request. May throw an exception. */
    virtual UdsResponse request(uint8_t sid, const uint8_t *data,
                                size_t size, bool throwNegative = true) = 0;

    /* All requests may throw an exception */
    /* Sends a DiagnosticSessionControl request. Returns parameter record. */
    std::vector<uint8_t> requestSession(uint8_t type);
    
    

    std::vector<uint8_t> requestSecuritySeed();

    void requestSecurityKey(const uint8_t *key, size_t size);

    /* ReadMemoryByAddress */
    std::vector<uint8_t> requestReadMemoryAddress(uint32_t address,
                                                  uint16_t length);
};
using UdsPtr = std::unique_ptr<Uds>;

} // namespace network
} // namespace lt

#endif
