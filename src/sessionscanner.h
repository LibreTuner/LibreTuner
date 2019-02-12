#ifndef SESSIONSCANNER_H
#define SESSIONSCANNER_H

#include <memory>

#include "asyncroutine.h"

namespace uds {
class Protocol;
using ProtocolPtr = std::unique_ptr<Protocol>;
}

// Scans for UDS sessions
class SessionScanner : public AsyncRoutine
{
public:
    using SuccessCallback = std::function<void(uint8_t)>;

    SessionScanner();

    void scan(uds::ProtocolPtr &&protocol);

    void onSuccess(SuccessCallback &&cb);

private:
    SuccessCallback success_;

    void callSuccess(uint8_t session);
};

#endif // SESSIONSCANNER_H
