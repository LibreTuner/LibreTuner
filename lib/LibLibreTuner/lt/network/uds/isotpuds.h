#ifndef LT_ISOTPUDS
#define LT_ISOTPUDS

#include "uds.h"
#include "network/isotp/isotp.h"

namespace lt {
namespace network {

class IsoTpUds : public Uds {
public:
	// Takes ownership of an ISO-TP interface
	IsoTpUds(IsoTpPtr&& isotp) : isotp_(std::move(isotp)) { assert(isotp_); }

	// Inherited via Uds
	virtual UdsResponse request(uint8_t sid, const uint8_t* data, size_t size) override;

private:
	IsoTpPtr isotp_;
};

}
}

#endif