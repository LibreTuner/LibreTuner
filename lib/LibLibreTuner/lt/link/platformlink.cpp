#include "platformlink.h"

#include "../network/uds/isotpuds.h"
#include "../network/can/canlog.h"
#include "../diagnostics/uds.h"
#include "../flash/mazdat1.h"
#include "../download/rmadownloader.h"

namespace lt {

network::CanPtr PlatformLink::can() {
	network::CanPtr can = datalink_.can(platform_.baudrate);
	if (!can) {
		throw std::runtime_error("CAN is unsupported with the selected datalink");
	}
	if (canLog_) {
        return std::make_unique<network::CanLogProxy>(std::move(can), canLog_);
	}
	return can;
}

network::IsoTpPtr PlatformLink::isotp() {
	network::IsoTpPtr isotp = datalink_.isotp();
	if (!isotp) {
		isotp = std::make_unique<network::IsoTp>(can());
	}
	isotp->setOptions(network::IsoTpOptions{platform_.serverId, platform_.serverId + 8});
	return isotp;
}

network::UdsPtr PlatformLink::uds() {
	return std::make_unique<network::IsoTpUds>(isotp());
}

DtcScannerPtr PlatformLink::dtcScanner() {
	return std::make_unique<UdsDtcScanner>(uds());
}

FlasherPtr PlatformLink::flasher() {
    if (platform_.flashMode == "mazdat1") {
		return std::make_unique<MazdaT1Flasher>(uds(), FlashOptions{platform_.flashAuthOptions});
	}
    throw std::runtime_error("invalid flash mode: " + platform_.flashMode);
}

download::DownloaderPtr PlatformLink::downloader() {
    if (platform_.downloadMode == "mazda23") {
        return std::make_unique<download::RMADownloader>(uds(), download::Options{platform_.downloadAuthOptions, platform_.romsize});
    }
    throw std::runtime_error("invalid download mode: " + platform_.downloadMode);
}

DataLoggerPtr PlatformLink::datalogger(DataLog &log) {
    if (platform_.logMode == "uds") {
        return std::make_unique<UdsDataLogger>(log, uds());
    }
    throw std::runtime_error("invalid log mode: " + platform_.logMode);
}

} // namespace lt
