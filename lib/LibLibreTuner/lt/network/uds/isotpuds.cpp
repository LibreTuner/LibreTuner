#include "isotpuds.h"

#include <sstream>

namespace lt {
namespace network {

UdsResponse IsoTpUds::request(uint8_t sid, const uint8_t* data, size_t size)
{
	IsoTpPacket packet;
	packet.append(&sid, 1);
	packet.append(data, size);

	isotp_->send(packet);
	// Receive until we get a non-response-pending packet
	while (true) {
		// Receive response
		IsoTpPacket res;
		isotp_->recv(res);

		if (res.empty()) {
			throw std::runtime_error("empty UDS response");
		}
		uint8_t id = res[0];

		if (id == UDS_RES_NEGATIVE) {
			if (res.size() >= 2) {
				uint8_t code = res[1];
				if (code == UDS_NRES_RCRRP) {
					// Response pending
					continue;
				}
				std::stringstream ss;
				ss << "negative UDS response: 0x" << std::hex << static_cast<int>(code)
					<< " (" << std::dec << static_cast<int>(code) << ")";
				throw std::runtime_error(ss.str());
			}
			throw std::runtime_error("negative UDS response");
		}

		if (id != sid + 0x40) {
			throw std::runtime_error(
				"uds response id (" + std::to_string(id) + ") does not match expected id (" + std::to_string(sid + 0x40) + ")");
		}
		UdsResponse response;
		response.data.assign(res.cbegin() + 1, res.cend());
		return response;
	}
}

}
}