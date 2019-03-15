#ifndef LT_PLATFORMLINK_H
#define LT_PLATFORMLINK_H

#include "datalink.h"
#include "../definition/platform.h"
#include "../network/network.h"
#include "../diagnostics/dtcscanner.h"
#include "../flash/flasher.h"
#include "../download/downloader.h"

namespace lt {
class PlatformLink {
public:
	PlatformLink(DataLink& datalink, const Platform &platform) : datalink_(datalink), platform_(platform) {}

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

private:
    DataLink &datalink_;
	const Platform &platform_;
};
} // namespace lt
#endif