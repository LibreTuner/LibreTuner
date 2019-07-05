#ifndef LT_UDSDTCSCANNER_H
#define LT_UDSDTCSCANNER_H

#include "dtcscanner.h"

#include "network/uds/uds.h"

namespace lt
{

class UdsDtcScanner : public DtcScanner
{
public:
    explicit UdsDtcScanner(network::UdsPtr && uds);

    virtual DiagnosticCodes scan() override;
    virtual DiagnosticCodes scanPending() override;

private:
    network::UdsPtr uds_;

    DiagnosticCodes scanPid(uint8_t pid = 0x3);
};

} // namespace lt

#endif