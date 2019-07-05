#ifndef LT_ISOTPUDS
#define LT_ISOTPUDS

#include "network/isotp/isotp.h"
#include "uds.h"

namespace lt
{
namespace network
{

class IsoTpUds : public Uds
{
public:
    // Takes ownership of an ISO-TP interface
    IsoTpUds(IsoTpPtr && isotp) : isotp_(std::move(isotp)) { assert(isotp_); }

    // Inherited via Uds
    virtual UdsPacket requestRaw(const UdsPacket & packet) override;
    virtual UdsPacket receiveRaw() override;

private:
    IsoTpPtr isotp_;
};

} // namespace network
} // namespace lt

#endif
