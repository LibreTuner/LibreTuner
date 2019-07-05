#ifndef LT_CANLOG_H
#define LT_CANLOG_H

#include "../../support/event.h"
#include "can.h"

#include <memory>
#include <vector>

namespace lt::network
{

enum class CanMessageDirection : bool
{
    Inbound,
    Outbound,
};

struct CanLogEntry
{
    CanMessageDirection direction;
    CanMessage message;
};

struct CanLog
{
    std::vector<CanLogEntry> messages_;

    inline std::size_t size() const noexcept { return messages_.size(); }

    template <typename T> inline void emplace_back(T && t)
    {
        messages_.emplace_back(std::forward<T>(t));
        eventAdded(messages_.back());
    }

    Event<CanLogEntry &> eventAdded;
};
using CanLogPtr = std::shared_ptr<CanLog>;

// Proxies a CAN interface and logs all sent and received messages
class CanLogProxy : public Can
{
public:
    CanLogProxy(CanPtr && can, CanLogPtr log)
        : can_(std::move(can)), log_(std::move(log))
    {
    }

    inline CanLogPtr log() const noexcept { return log_; }
    inline void setLog(CanLogPtr log) noexcept { log_ = std::move(log); }

    void send(const CanMessage & message) override
    {
        can_->send(message);
        if (log_)
        {
            log_->emplace_back(
                CanLogEntry{CanMessageDirection::Outbound, message});
        }
    }

    bool recv(CanMessage & message, std::chrono::milliseconds timeout) override
    {
        bool res = can_->recv(message, timeout);
        if (res && log_)
        {
            log_->emplace_back(
                CanLogEntry{CanMessageDirection::Inbound, message});
        }
        return res;
    }

    void clearBuffer() noexcept override { can_->clearBuffer(); }

private:
    CanPtr can_;
    CanLogPtr log_;
};

} // namespace lt::network

#endif // LIBRETUNER_CANLOG_H
