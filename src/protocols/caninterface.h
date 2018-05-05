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

#ifndef CANINTERFACE_H
#define CANINTERFACE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>


struct CanMessage
{
public:
    void setMessage(uint32_t id, const uint8_t *message, uint8_t length);
    
    CanMessage();
    CanMessage(uint32_t id, const uint8_t *message, uint8_t length);
    
    uint32_t id() const
    {
        return id_;
    }
    
    const uint8_t *message() const
    {
        return message_;
    }
    
    /* Returns a human-readable string representing the message data */
    std::string strMessage() const;
    
    uint8_t length() const
    {
        return messageLength_;
    }
    
    uint8_t &operator[](uint8_t index)
    {
        return message_[index];
    }
    
    const uint8_t &operator[](uint8_t index) const
    {
        return message_[index];
    }
    
    uint32_t id_;
    uint8_t message_[8];
    uint8_t messageLength_;
};



class CanInterface;
typedef std::shared_ptr<CanInterface> CanInterfacePtr;



/* Abstract CAN interface */
class CanInterface
{
public:
    enum CanError
    {
        ERR_SUCCESS = 0,
        ERR_SOCKET, // Socket creation error. err will be set
        ERR_READ, // Read error. err will be set
        ERR_WRITE,
    };
    
    class Callbacks
    {
    public:
        /* Called when a new message is received */
        virtual void onRecv(const CanMessage &message) =0;
        
        /* Called when an error occurs. In some cases, err
         * will be set to an errno value. */
        virtual void onError(CanError error, int err) =0;
    };
    
    CanInterface(Callbacks *callbacks = nullptr);
    virtual ~CanInterface() {};
    
    /* Send a CAN message. len should be no greater than 8.
     * Returns true if a message was sent */
    bool send(int id, const uint8_t *message, uint8_t len);
    
    virtual bool send(const CanMessage &message) =0;
    
    
    /* Receives a message and returns the size of the message.
     * message should be at least 8 bytes wide. Returns false
     * if an error occured while reading. */
    virtual bool recv(CanMessage &message) =0;
    
    /* Returns a human-readable string representing an error code */
    static std::string strError(CanError error, int err = 0);
    
    /* Returns a human-readable string representing the last error */
    std::string strError();
    
    /* Returns the last error. Returns ERR_SUCCESS if there
     * have been no errors. */
    CanError lastError() const
    {
        return lastError_;
    }
    
    int lastErrno() const
    {
        return lastErrno_;
    }
    
    void addCallbacks(Callbacks *callbacks)
    {
        callbacks_.push_back(callbacks);
    }
    
    void removeCallbacks(Callbacks *callbacks);
    
    /* Returns true if the socket is ready for reading/writing */
    virtual bool valid() =0;
    
    /* Starts reading from the interface and calling callbacks. This function
     * may block until the the interface has fully started. */
    virtual void start() =0;
    
protected:
    /* Calls the onRecv callbacks */
    void callOnRecv(const CanMessage &message);
    
    /* Calls the onError callbacks */
    void callOnError(CanError error, int err);
    
    std::vector<Callbacks*> callbacks_;
    CanError lastError_;
    int lastErrno_;
};

#endif // CANINTERFACE_H
