#ifndef LT_ISOTPELM_H
#define LT_ISOTPELM_H

#include "../command/elm327.h"
#include "isotp.h"

#include <queue>

namespace lt::network
{

class IsoTpElm : public IsoTp
{
public:
    // Constructs from an opened Elm327 device
    explicit IsoTpElm(Elm327Ptr device, IsoTpOptions options = IsoTpOptions());

    void recv(IsoTpPacket & result) override;

    void request(const IsoTpPacket & req, IsoTpPacket & result) override;

    void send(const IsoTpPacket & packet) override;

    void setOptions(const IsoTpOptions & options) override;

    // Updates header and receive ids
    void updateOptions();

private:
    Elm327Ptr device_;
    IsoTpOptions options_;

    std::queue<IsoTpPacket> buffer_;

    // Extracts packets from ELM327 response and fills buffer
    void processResponse(std::vector<std::string> & response);
};

} // namespace lt::network

#endif // LIBRETUNER_ISOTPELM_H
