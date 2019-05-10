#ifndef LT_PLATFORMLINK_H
#define LT_PLATFORMLINK_H

#include "../datalog/datalogger.h"
#include "../definition/platform.h"
#include "../diagnostics/dtcscanner.h"
#include "../download/downloader.h"
#include "../flash/flasher.h"
#include "../network/network.h"
#include "datalink.h"

namespace lt {
namespace network {
class CanLog;
using CanLogPtr = std::shared_ptr<CanLog>;
} // namespace network

class PlatformLink {
public:
    PlatformLink(DataLink &datalink, const Platform &platform)
        : datalink_(datalink), platform_(platform) {}

    inline const DataLink &datalink() const noexcept { return datalink_; }

    inline const Platform &platform() const noexcept { return platform_; }

    // All network methods throw an exception if a protocol
    // is unsupported. That is, each method is guaranteed to either
    // throw an exception or return a valid interface.
    network::CanPtr can();
    network::IsoTpPtr isotp();
    network::UdsPtr uds();

    DtcScannerPtr dtcScanner();
    FlasherPtr flasher();
    download::DownloaderPtr downloader();
    DataLoggerPtr datalogger(DataLog &log);

    inline void setCanLog(network::CanLogPtr log) noexcept {
        canLog_ = std::move(log);
    }

private:
    DataLink &datalink_;
    const Platform &platform_;
    network::CanLogPtr canLog_;
};

} // namespace lt
#endif
