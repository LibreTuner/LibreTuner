#ifndef LT_UDSINFO_H
#define LT_UDSINFO_H

#include <network/uds/uds.h>
#include <string>
#include <support/types.h>

namespace lt
{

enum class ScanPids : unsigned
{
    None = 0,
    VIN = 0x1,
    CalibrationID = 0x2,
    ECUName = 0x4,
};
ENABLE_BITMASK(ScanPids)

// Service 09 PIDs
constexpr uint8_t OBD_REQ_VIN = 0x2;
constexpr uint8_t OBD_REQ_CAL = 0x4; // Calibration ID
constexpr uint8_t OBD_REQ_ECUNAME = 0x0A;

/* Scans OBD-II vehicle information PIDs. */
class UdsInfo
{
public:
    explicit UdsInfo(network::UdsPtr && uds);

    // Scans selected PIDs. Get result with getter methods (vin(), etc.)
    void scan();

    // Scans individual PIDs
    std::string scanVin();
    std::string scanCalibrationId();
    std::string scanEcuName();

    // Sets the scan PIDs. By default, all PIDs are set
    inline void setPids(ScanPids pids) noexcept { pids_ = pids; }

private:
    network::UdsPtr uds_;

    ScanPids pids_{ScanPids::VIN | ScanPids::CalibrationID | ScanPids::ECUName};

    // Variables from last scan
    std::string vin_;
    std::string calibrationId_;
    std::string ecuName_;

    std::string scanString(uint8_t pid);
};
} // namespace lt

#endif // LT_UDSINFO_H
