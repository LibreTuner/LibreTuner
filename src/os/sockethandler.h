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

#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class Socket {
public:
    /* Returns the file descriptor for the socket */
    virtual int fd() = 0;

    /* Called when the socket can be read */
    virtual void onRead() = 0;
};

/**
 * Handles asynchronous sockets on *nix hosts
 */
class SocketHandler {
public:
    /* Returns the global SocketHandler object */
    static SocketHandler *get();

    void addSocket(Socket *socket);
    void removeSocket(Socket *socket);

    /* Starts the thread for handling socket reads/writes */
    void initialize();

    ~SocketHandler();

private:
    std::thread thread_;
    std::vector<Socket *> sockets_;

    /* Condition variable and mutex used for addSocket() blocking */
    std::condition_variable cv_;
    std::mutex cv_m_;

    std::mutex mutex_;
    /* Set to true after select() returns in run() */
    bool runLooped_;

    void run();
    SocketHandler();

    bool running_;
};

#endif // SOCKETHANDLER_H
