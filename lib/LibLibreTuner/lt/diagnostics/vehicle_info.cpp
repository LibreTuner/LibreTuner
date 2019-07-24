#include "vehicle_info.h"

#include <array>
#include <cassert>

namespace lt
{
vehicle_info request_vehicle_info(network::Uds & uds, ScanPids pids)
{
    vehicle_info info{};
    if ((pids & ScanPids::VIN) != ScanPids::None)
        info.vin = request_service9_string(uds, OBD_REQ_VIN);
    if ((pids & ScanPids::CalibrationID) != ScanPids::None)
        info.calibration_id = request_service9_string(uds, OBD_REQ_CAL);
    if ((pids & ScanPids::ECUName) != ScanPids::None)
        info.ecu_name = request_service9_string(uds, OBD_REQ_ECUNAME);
    return info;
}

std::string request_service9_string(network::Uds & uds, uint8_t pid)
{
    std::array<uint8_t, 1> request = {pid};
    network::UdsPacket response =
        uds.requestRaw(network::UdsPacket(9, request.data(), request.size()));
    if (response.negative())
        return std::string();

    return std::string(reinterpret_cast<const char *>(response.data.data()),
                       response.data.size());
}
} // namespace lt
