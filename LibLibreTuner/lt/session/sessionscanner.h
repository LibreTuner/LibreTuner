#ifndef SESSIONSCANNER_H
#define SESSIONSCANNER_H

#include <memory>

#include "../network/uds/uds.h"
#include "../support/asyncroutine.h"

namespace lt
{

// Scans for UDS sessions
class SessionScanner : public AsyncRoutine
{
public:
    using SuccessCallback = std::function<void(uint8_t)>;

    SessionScanner();

    void scan(network::Uds & protocol, uint8_t minimum = 0,
              uint8_t maximum = 0xFF);

    void onSuccess(SuccessCallback && cb);

private:
    SuccessCallback success_;

    void callSuccess(uint8_t session);
};

} // namespace lt

#endif // SESSIONSCANNER_H
