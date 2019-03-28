#include "isotpuds.h"

namespace lt {
namespace network {

UdsPacket IsoTpUds::requestRaw(const UdsPacket &packet) {
    IsoTpPacket isotpPacket;
    isotpPacket.append(&packet.code, 1);
    isotpPacket.append(packet.data.data(), packet.data.size());
    isotp_->send(isotpPacket);

    return receiveRaw();
}

UdsPacket IsoTpUds::receiveRaw()
{
    IsoTpPacket res;
    isotp_->recv(res);

    std::vector<uint8_t> data;
    res.moveInto(data);
    return UdsPacket(data.data(), data.size());
}

}
}
