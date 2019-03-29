/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SIGNAL_H
#define SIGNAL_H

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

namespace lt {

template <typename Func> class Signal;

template <typename Func> class Connection {
public:
    using SignalType = Signal<Func>;
    Connection(const std::shared_ptr<SignalType> &signal, Func f)
        : signal_(signal), func_(f) {}
    
    Connection() {}

    ~Connection() { disconnect(); }

    template <typename... Args> typename Func::result_type call(Args... args) {
        return func_(std::forward<Args>(args)...);
    }

    void disconnect() {
        if (signal_) {
            signal_->disconnect(this);
            signal_.reset();
        }
    }

    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;
    Connection(Connection &&) = delete;
    Connection &operator=(Connection &&) = delete;

private:
    std::shared_ptr<SignalType> signal_;
    Func func_;
};

template <typename Func> class Signal {
public:
    using ConnectionType = Connection<Func>;
    using ConnectionPtr = std::shared_ptr<ConnectionType>;

    std::vector<std::weak_ptr<ConnectionType>> connections;

    /* Connects a new listener and returns the connection */
    std::shared_ptr<ConnectionType> connect(Func f) {
        auto self = self_.lock();
        assert(self && "Failed to lock self. Is this signal a share_ptr?");
        auto conn = std::make_shared<ConnectionType>(self, f);
        connections.push_back(conn);
        return conn;
    }

    /* Calls all connections */
    template <typename... Args> void call(Args... args) {
        for (const auto &conn : connections) {
            if (auto sptr = conn.lock()) {
                sptr->call(std::forward<Args>(args)...);
            }
        }
    }

    static std::shared_ptr<Signal<Func>> create() {
        auto signal = std::make_shared<Signal<Func>>();
        signal->self_ = signal;
        return signal;
    }

    /* Returns the amount of registered connections */
    size_t count() const;

private:
    friend ConnectionType;

    /* Disconnects a connection from the signal */
    void disconnect(ConnectionType *con) {
        std::for_each(connections.begin(), connections.end(),
                      [con](std::weak_ptr<ConnectionType> &conn) {
                          if (auto sptr = conn.lock()) {
                              if (sptr.get() == con) {
                                  conn.reset();
                              }
                          }
                      });
    }

    std::weak_ptr<Signal<Func>> self_;
};

template <typename Func> size_t Signal<Func>::count() const {
    size_t c = 0;
    std::for_each(connections.begin(), connections.end(),
                  [&c](const std::weak_ptr<ConnectionType> &con) {
                      if (con) {
                          c++;
                      }
                  });
    return c;
}

}

#endif // SIGNAL_H
