#ifndef LT_UDSDTCSCANNER_H
#define LT_UDSDTCSCANNER_H

#include "dtcscanner.h"

#include "network/uds/uds.h"

namespace lt {

class UdsDtcScanner : public DtcScanner {
  public:
    explicit UdsDtcScanner(network::UdsPtr &&uds);

    virtual void scan(DiagnosticCodes &result) override;
    virtual void scanPending(DiagnosticCodes &result) override;

  private:
    network::UdsPtr uds_;

    void scanPid(DiagnosticCodes &result, uint8_t pid = 0x3);
};

} // namespace lt

#endif