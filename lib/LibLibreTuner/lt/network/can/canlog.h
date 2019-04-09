#ifndef LT_CANLOG_H
#define LT_CANLOG_H

#include "can.h"

#include <vector>
#include <memory>

namespace lt::network {

class CanLog {

};

using CanLogPtr = std::shared_ptr<CanLog>;

// Proxies a CAN interface and logs all sent and received messages
class CanLogProxy : public Can {
public:
    CanLogProxy(CanPtr &&can, CanLogPtr log) : can_(std::move(can)), log_(std::move(log)) {}

    inline CanLogPtr log() const noexcept { return log_; }
    inline void setLog(CanLogPtr log) noexcept { log_ = std::move(log); }

    void send(const CanMessage &message) override {
        can_->send(message);
    }

    bool recv(CanMessage &message, std::chrono::milliseconds timeout) override {
        return can_->recv(message, std::move(timeout));
    }

    void clearBuffer() noexcept override {
        can_->clearBuffer();
    }


private:
    CanPtr can_;
    CanLogPtr log_;
};

} // namespace lt::network


#endif //LIBRETUNER_CANLOG_H
