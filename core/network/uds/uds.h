#ifndef LT_UDS_H
#define LT_UDS_H

#include <cstdint>
#include <memory>
#include <vector>

namespace lt
{
namespace network
{

/* Request SIDs */
constexpr uint8_t UDS_REQ_SESSION = 0x10;
constexpr uint8_t UDS_REQ_SECURITY = 0x27;
constexpr uint8_t UDS_REQ_READMEM = 0x23;
constexpr uint8_t UDS_REQ_REQUESTDOWNLOAD = 0x34;
constexpr uint8_t UDS_REQ_REQUESTUPLOAD = 0x35;
constexpr uint8_t UDS_REQ_TRANSFERDATA = 0x36;
constexpr uint8_t UDS_REQ_READBYID = 0x22;

constexpr uint8_t UDS_RES_NEGATIVE = 0x7F;

/* Negative response codes */
// requestCorrectlyReceivedResponsePending
constexpr uint8_t UDS_NRES_RCRRP = 0x78;

struct UdsPacket
{
    std::vector<uint8_t> data;
    uint8_t code{0};

    UdsPacket(const uint8_t * raw, std::size_t size)
    {
        if (size == 0)
        {
            return;
        }
        code = raw[0];
        data.resize(size - 1);
        std::copy(raw + 1, raw + size, data.begin());
    }

    UdsPacket(uint8_t _code, const uint8_t * payload, std::size_t size)
        : data(payload, payload + size), code(_code)
    {
    }

    UdsPacket() = default;

    bool empty() const noexcept { return data.empty() && code == 0; }

    bool negative() const noexcept { return code == UDS_RES_NEGATIVE; }
    uint8_t negativeCode() const noexcept { return data.size() > 1 ? data[1] : 0; }
};

class Uds
{
public:
    virtual ~Uds() = default;

    /* Sends a request. May throw an exception. Throws an
       exception if a negative response is received. (Not
       including RCRRP). */
    UdsPacket request(uint8_t sid, const uint8_t * data, size_t size);

    /* All requests may throw an exception */
    /* Sends a DiagnosticSessionControl request. Returns parameter record. */
    std::vector<uint8_t> requestSession(uint8_t type);

    std::vector<uint8_t> requestSecuritySeed();

    void requestSecurityKey(const uint8_t * key, size_t size);

    /* ReadMemoryByAddress */
    std::vector<uint8_t> requestReadMemoryAddress(uint32_t address,
                                                  uint16_t length);

    std::vector<uint8_t> readDataByIdentifier(uint16_t id);

    // Sends a request but does not throw an exception on negative errors.
    // Must not handle RCRRP or other negative responses.
    virtual UdsPacket requestRaw(const UdsPacket & packet) = 0;

    virtual UdsPacket receiveRaw() = 0;
};
using UdsPtr = std::unique_ptr<Uds>;

} // namespace network
} // namespace lt

#endif
