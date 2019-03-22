#include "sessionscanner.h"
#include "../network/uds/uds.h"

namespace lt {

SessionScanner::SessionScanner()
{

}


void SessionScanner::scan(network::Uds &protocol, uint8_t minimum, uint8_t maximum)
{
    for (int session = minimum; session <= maximum; ++session) {
        notifyProgress(static_cast<float>(session - minimum) / (maximum - minimum));
        
        uint8_t sessionByte = static_cast<uint8_t>(session);
        network::UdsResponse res = protocol.request(network::UDS_REQ_SESSION, &sessionByte, 1, false);
        if (!res.negative) {
            callSuccess(static_cast<uint8_t>(session));
        }
    }
}


void SessionScanner::onSuccess(SessionScanner::SuccessCallback &&cb)
{
    success_ = std::move(cb);
}


void SessionScanner::callSuccess(uint8_t session)
{
    if (success_) {
        success_(session);
    }
}

}

