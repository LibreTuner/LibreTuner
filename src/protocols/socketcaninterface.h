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

#ifndef SOCKETCANINTERFACE_H
#define SOCKETCANINTERFACE_H

#include "../os/sockethandler.h"
#include "caninterface.h"
#include <mutex>
#include <thread>

/*
 * SocketCan interface for linux hosts
 */
class SocketCanInterface : public CanInterface {
public:
    SocketCanInterface(SocketCanInterface &) = delete;
    SocketCanInterface(const SocketCanInterface &) = delete;
    SocketCanInterface(SocketCanInterface &&) = delete;

    ~SocketCanInterface() override;


    /* Closes the socket */
    void close();

    /* Binds the socket to a SocketCAN interface. Returns false
     * if an error occured. */
    bool bind(const std::string &ifname);

    /* Sets the socket as nonblocking for async operations */
    void setNonblocking();

    /* Socket functions */
    int fd() const;

    // These constructors should never be used directly!
    SocketCanInterface(const std::string &ifname);

    // CanInterface interface
public:
    virtual void send(const CanMessage &message) override;

    /* Returns true if a message was received before the timeoutc or if
     * nonblocking is enabled and no messages are queued. */
    virtual bool recv(CanMessage &message,
                      std::chrono::milliseconds timeout) override;

    /* Returns true if the socket is ready for reading/writing */
    bool valid() override { return socket_ > 0; }

    /* Starts the read loop thread for asynchronous operations */
    void start() override;

private:
    int socket_ = 0;
};

#endif // SOCKETCANINTERFACE_H
