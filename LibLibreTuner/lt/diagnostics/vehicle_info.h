#ifndef LT_UDSINFO_H
#define LT_UDSINFO_H

#include "../network/uds/uds.h"
#include <string>
#include "../support/types.h"

namespace lt
{

enum class ScanPids : unsigned
{
    None = 0,
    VIN = 0x1,
    CalibrationID = 0x2,
    ECUName = 0x4,
    All = VIN | CalibrationID | ECUName,
};
ENABLE_BITMASK(ScanPids)

// Service 09 PIDs
constexpr uint8_t OBD_REQ_VIN = 0x2;
constexpr uint8_t OBD_REQ_CAL = 0x4; // Calibration ID
constexpr uint8_t OBD_REQ_ECUNAME = 0x0A;

struct vehicle_info
{
    std::string vin;
    std::string calibration_id;
    std::string ecu_name;
};

std::string request_service9_string(network::Uds & uds, uint8_t pid);

/* Scans vehicle information. Unscanned and unavailable information
 * is left empty. */
vehicle_info request_vehicle_info(network::Uds & uds, ScanPids pids);

} // namespace lt

#endif // LT_UDSINFO_H
