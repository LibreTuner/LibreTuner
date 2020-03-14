#ifndef LIBRETUNER_ISOTPJ2534_H
#define LIBRETUNER_ISOTPJ2534_H

#include "isotp.h"
#include "j2534/j2534.h"

namespace lt::network
{

// ISO 15765-2 transport layer (ISO-TP) for sending large packets over CAN
class IsoTpJ2534 : public IsoTp
{
public:
    // `device` MUST NOT be empty
    explicit IsoTpJ2534(j2534::DevicePtr device,
        IsoTpOptions options = IsoTpOptions());

    void recv(IsoTpPacket & result) override;

    void request(const IsoTpPacket & req, IsoTpPacket & result) override;

    void send(const IsoTpPacket & packet) override;

    void setOptions(const IsoTpOptions & options) override
    {
        options_ = options;
    }

private:
    j2534::DevicePtr device_;
    j2534::Channel channel_;
    IsoTpOptions options_;
};

}



#endif //LIBRETUNER_ISOTPJ2534_H
