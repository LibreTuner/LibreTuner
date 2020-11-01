#include "uds.h"

#include <array>
#include <sstream>
#include <stdexcept>

namespace lt
{
namespace network
{

UdsPacket Uds::request(uint8_t sid, const uint8_t * data, size_t size)
{
    // Receive until we get a non-response-pending packet

    // Build request
    UdsPacket request(sid, data, size);

    UdsPacket response = requestRaw(request);

    do
    {
        if (response.negative())
        {
            uint8_t code = response.negativeCode();
            if (code == UDS_NRES_RCRRP)
            {
                // Response pending
                response = receiveRaw();
                continue;
            }
            std::stringstream ss;
            ss << "negative UDS response: 0x" << std::hex
               << static_cast<int>(code) << " (" << std::dec
               << static_cast<int>(code) << ")";
            throw std::runtime_error(ss.str());
        }

        if (response.code != sid + 0x40)
        {
            throw std::runtime_error("uds response id (" +
                                     std::to_string(response.code) +
                                     ") does not match expected id (" +
                                     std::to_string(sid + 0x40) + ")");
        }
        return response;
    } while (true);
}

std::vector<uint8_t> Uds::requestSession(uint8_t type)
{
    UdsPacket res = request(UDS_REQ_SESSION, &type, 1);
    if (res.data.empty())
        {
        throw std::runtime_error("received empty session control response");
    }

    if (res.data[0] != type)
    {
        throw std::runtime_error("diagnosticSessionType mismatch");
    }

    res.data.erase(res.data.begin());
    return res.data;
}

std::vector<uint8_t> Uds::requestSecuritySeed()
{
    uint8_t req[] = {1};
    UdsPacket res = request(UDS_REQ_SECURITY, req, 1);
    if (res.data.empty())
    {
        throw std::runtime_error("received empty security access packet");
    }

    if (res.data[0] != req[0])
    {
        throw std::runtime_error("securityAccessType mismatch");
    }

    res.data.erase(res.data.begin(), res.data.begin() + 1);
    return res.data;
}

void Uds::requestSecurityKey(const uint8_t * key, size_t size)
{
    std::vector<uint8_t> req(size + 1);
    req[0] = 2;
    std::copy(key, key + size, req.data() + 1);

    UdsPacket res = request(UDS_REQ_SECURITY, req.data(), req.size());
    if (res.data.empty())
    {
        throw std::runtime_error("received empty security access response");
    }
}

std::vector<uint8_t> Uds::requestReadMemoryAddress(uint32_t address,
                                                   uint16_t length)
{
    std::array<uint8_t, 6> req;
    req[0] = (address & 0xFF000000) >> 24;
    req[1] = (address & 0xFF0000) >> 16;
    req[2] = (address & 0xFF00) >> 8;
    req[3] = address & 0xFF;

    req[4] = length >> 8;
    req[5] = length & 0xFF;

    UdsPacket res = request(UDS_REQ_READMEM, req.data(), req.size());

    return res.data;
}

std::vector<uint8_t> Uds::readDataByIdentifier(uint16_t id)
{
    std::array<uint8_t, 2> req;
    req[0] = id >> 8;
    req[1] = id & 0xFF;

    UdsPacket res = request(UDS_REQ_READBYID, req.data(), req.size());
    return res.data;
}

} // namespace network
} // namespace lt
