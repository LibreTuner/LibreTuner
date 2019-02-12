#include "sessionscanner.h"
#include "protocols/udsprotocol.h"

SessionScanner::SessionScanner()
{

}


void SessionScanner::scan(uds::ProtocolPtr &&protocol, uint8_t minimum, uint8_t maximum)
{
    for (int session = minimum; session <= maximum; ++session) {
        try {
            protocol->requestSession(static_cast<uint8_t>(session));
            callSuccess(static_cast<uint8_t>(session));
        } catch (const std::runtime_error &err) {
            // Ignore exception
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

