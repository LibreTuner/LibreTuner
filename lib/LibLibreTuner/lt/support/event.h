#ifndef LT_EVENT_H
#define LT_EVENT_H

#include <forward_list>
#include <functional>
#include <memory>

namespace lt {

template <typename... Args> class EventState;

template <typename... Args> class EventConnection {
public:
    using Func = std::function<void(Args...)>;
    using State = EventState<Args...>;

    template <typename F>
    EventConnection(F &&func, std::weak_ptr<State> &&state)
        : callback_(std::forward<F>(func)), state_(std::move(state)) {}

    // Disconnects from the event
    void disconnect() noexcept {
        /*if (auto state = state_.lock()) {
            state->disconnect(this);
        };*/
        state_.reset();
    }

    template <typename... A> void operator()(A &&... args) const {
        callback_(std::forward<A>(args)...);
    }

private:
    Func callback_;
    std::weak_ptr<State> state_;
};

template <typename... Args> class EventState {
public:
    using Connection = EventConnection<Args...>;

    template <typename... A> void dispatch(A &&... args) const {
        for (const std::weak_ptr<Connection> &connPtr : connections_) {
            if (auto conn = connPtr.lock()) {
                (*conn)(std::forward<A>(args)...);
            }
        }
    }

    // Removes expired connections
    void removeExpired() noexcept {
        connections_.remove_if([](auto &conn) { return conn.expired(); });
    }

    // Adds a connection to the dispatch list
    void add(std::weak_ptr<Connection> conn) noexcept {
        connections_.emplace_front(std::move(conn));
    }

    // Disconnects a connection
    void disconnect(Connection *connection) noexcept {
        connections_.remove_if([connection](auto &c) {
            if (auto s = c.lock()) {
                return s.get() == connection;
            }
            return true; // connection has expired
        });
    }

private:
    std::forward_list<std::weak_ptr<Connection>> connections_;
};

template <typename... Args> class Event {
public:
    using State = EventState<Args...>;
    using Connection = typename State::Connection;
    using ConnectionPtr = std::shared_ptr<Connection>;

    Event() : state_(std::make_shared<State>()) {}

    // Creates a new connection with a callback
    template <typename Func> ConnectionPtr connect(Func &&f) noexcept {
        ConnectionPtr conn =
            std::make_shared<Connection>(std::forward<Func>(f), state_);
        state_->add(conn);
        return conn;
    }

    template <typename... A> void operator()(A &&... args) {
        state_->dispatch(std::forward<A>(args)...);
    }

private:
    std::shared_ptr<State> state_;
};

} // namespace lt

#endif
