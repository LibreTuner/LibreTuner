#ifndef SOCKETCAN_H
#define SOCKETCAN_H

#include "can.h"
#include "os/socket.h"

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <string>
#include <thread>

#ifdef WITH_SOCKETCAN

namespace lt
{
namespace network
{

class SocketCanReceiver
{
public:
    SocketCanReceiver(os::Socket & socket) : socket_(socket) {}

    ~SocketCanReceiver();

    // Returns the first message in the buffer and waits if empty.
    // If the worker thread has thrown an exception, passes it here.
    bool recv(CanMessage & message, std::chrono::milliseconds timeout);

    void start();
    void stop();

    void clearBuffer();

private:
    os::Socket & socket_;

    void work();

    std::thread receiver_;
    std::condition_variable received_;
    std::atomic<bool> stop_{false};
    std::atomic<bool> running_{false};
    std::future<void> result_;
    std::mutex mutex_;

    CanMessageBuffer buffer_;
};

class SocketCan : public Can
{
public:
    SocketCan(SocketCan &) = delete;
    SocketCan(const SocketCan &) = delete;
    SocketCan(SocketCan &&) = delete;

    ~SocketCan() override;

    SocketCan(const std::string & ifname);

    // Can interface
public:
    virtual void send(const CanMessage & message) override;

    /* Returns false if the timeout expired and no message was read. */
    virtual bool recv(CanMessage & message,
                      std::chrono::milliseconds timeout) override;

    virtual void clearBuffer() noexcept override;

private:
    os::Socket socket_;
    SocketCanReceiver receiver_;
};

} // namespace network
} // namespace lt

#endif

#endif // SOCKETCAN_H
