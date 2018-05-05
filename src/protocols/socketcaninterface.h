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

#include <thread>
#include "caninterface.h"
#include "../os/sockethandler.h"

/*
 * SocketCan interface for linux hosts
 */
class SocketCanInterface : public CanInterface, public Socket
{
public:
    SocketCanInterface(CanInterface::Callbacks *callbacks = nullptr);
    /* Creates a socket and attempts to bind to an interface.
     * For errors, check valid() and lastError() */
    SocketCanInterface(CanInterface::Callbacks *callbacks, const std::string &ifname);
    
    ~SocketCanInterface();
    
    bool send(const CanMessage &message) override;
    
    bool recv(CanMessage &message) override;
    
    /* Closes the socket */
    void close();
    
    /* Starts the read loop thread for asynchronous operations */
    void start() override;
    
    /* Binds the socket to a SocketCAN interface. Returns false
     * if an error occured. */
    bool bind(const std::string &ifname);
    
    /* Returns true if the socket is ready for reading/writing */
    bool valid() override
    {
        return socket_ > 0;
    }
    
    /* Sets the socket as nonblocking for async operations */
    void setNonblocking();
    
    /* Socket functions */
    int fd() override;
    void onRead() override;
    
private:
    int socket_ = 0;
};

#endif // SOCKETCANINTERFACE_H
