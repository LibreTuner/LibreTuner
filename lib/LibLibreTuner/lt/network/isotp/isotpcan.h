#ifndef LT_ISOTPCAN_H
#define LT_ISOTPCAN_H

#include "isotp.h"

namespace lt::network {

// ISO 15765-2 transport layer (ISO-TP) for sending large packets over CAN
class IsoTpCan : public IsoTp {
public:
    // Takes ownership of a CAN interface
    explicit IsoTpCan(CanPtr &&can = CanPtr(), IsoTpOptions options = IsoTpOptions());
    ~IsoTpCan();

    void recv(IsoTpPacket &result) override;

    // Sends a request and waits for a response
    void request(const IsoTpPacket &req, IsoTpPacket &result) override;

    void send(const IsoTpPacket &packet) override;

    inline void setCan(CanPtr &&can) { can_ = std::move(can); }

    // May return nullptr
    inline Can *can() { return can_.get(); }

    void setOptions(const IsoTpOptions &options) override { options_ = options; }

    inline const IsoTpOptions &options() const { return options_; }

    // Receives next CAN message with proper id
    CanMessage recvNextFrame();
    CanMessage recvNextFrame(uint8_t expectedType);

private:
    CanPtr can_;
    IsoTpOptions options_;

    void sendSingleFrame(const uint8_t *data, std::size_t size);
};
}


#endif //LIBRETUNER_ISOTPCAN_H
