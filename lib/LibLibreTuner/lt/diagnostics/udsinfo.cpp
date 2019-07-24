#include "udsinfo.h"

#include <array>
#include <cassert>

namespace lt
{

UdsInfo::UdsInfo(lt::network::UdsPtr && uds) : uds_(std::move(uds))
{
    assert(uds_);
}

std::string UdsInfo::scanVin() { return scanString(OBD_REQ_VIN); }

std::string UdsInfo::scanCalibrationId() { return scanString(OBD_REQ_CAL); }

std::string UdsInfo::scanEcuName() { return scanString(OBD_REQ_ECUNAME); }

inline std::string UdsInfo::scanString(uint8_t pid)
{
    std::array<uint8_t, 1> request = {pid};
    network::UdsPacket response =
        uds_->requestRaw(network::UdsPacket(9, request.data(), request.size()));
    if (response.negative())
        return "Unavailable";

    return std::string(reinterpret_cast<const char *>(response.data.data()),
                       response.data.size());
}

void UdsInfo::scan() {
    if ((pids_ & ScanPids::VIN) != ScanPids::None)
        vin_ = scanVin();
    if ((pids_ & ScanPids::CalibrationID) != ScanPids::None)
        calibrationId_ = scanCalibrationId();
    if ((pids_ & ScanPids::ECUName) != ScanPids::None)
        ecuName_ = scanEcuName();
}

} // namespace lt
