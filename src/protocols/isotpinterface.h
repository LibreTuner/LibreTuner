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

#ifndef ISOTPINTERFACE_H
#define ISOTPINTERFACE_H

#include <memory>
#include <vector>
#include <thread>
#include <condition_variable>

#include <QObject>
#include <QTimer>

#include "caninterface.h"


class IsoTpMessage
{
public:
    IsoTpMessage(int id);
    
    const uint8_t *message() const
    {
        return message_.data();
    }
    
    size_t length() const
    {
        return message_.size();
    }
    
    int id() const
    {
        return id_;
    }
    
    /* Appends data to the message */
    void append(const uint8_t *message, size_t length);
    
private:
    std::vector<uint8_t> message_;
    int id_;
};


typedef std::shared_ptr<IsoTpMessage> IsoTpMessagePtr;




class IsoTpInterface : public CanInterface::Callbacks
{
public:
    enum IsoTpError {
        ERR_SUCCESS = 0,
        ERR_BUSY, // The interface is already processing a request
        ERR_CAN,
        ERR_SIZE, // The CAN frame size was too short
        ERR_CONSEC, // Incorrect consecutive index
    };
    
    class Callbacks {
    public:
        virtual void onRecv(IsoTpMessagePtr message) =0;
        
        virtual void onCanError(CanInterface::CanError error, int err) =0;
        
        virtual void onError(IsoTpError error) =0;
        
        virtual void onTimeout() =0;
    };
    
    
    IsoTpInterface(Callbacks *callbacks, std::shared_ptr<CanInterface> can, int srcId = 0x7DF, int dstId = 0x7E8);
    
    ~IsoTpInterface();
    
    /* Sends an isotp request. The response will be sent through
     * the onRecv() callback or onTimeout() will be called if
     * a response wasn't received in time. The timeout unit is milliseconds.
     * Returns false if a request is already being processed. */
    IsoTpError request(const uint8_t *message, size_t length, int timeout=200);
    
    static std::string strError(IsoTpError error);
    
    /* CAN callbacks */
    void onError(CanInterface::CanError error, int err) override;
    void onRecv(const CanMessage & message) override;
    
    /* Returns true if the interface is valid */
    bool valid() const
    {
        return canInterface_->valid();
    } 
    
    CanInterface *canInterface()
    {
        return canInterface_.get();
    }
    
    enum State
    {
        STATE_NONE, // Waiting for a request
        STATE_RESP, // Waiting for a single/first frame
        STATE_CONSEC, // Waiting for a consecutive frame
        STATE_FLOW, // Waiting for a flow control frame
    };
    
private:
    std::shared_ptr<CanInterface> canInterface_;
    int srcId_, dstId_;
    Callbacks *callbacks_;
    
    /* Data being sent */
    std::vector<uint8_t> msg_;
    std::vector<uint8_t>::iterator msgPtr_;
    
    /* The message being received */
    IsoTpMessagePtr recvMsg_;
    /* Amount of bytes remaining to be received */
    size_t recvRemaining_;
    
    /* Consecutive index. Resets at 15. Used to both
     * sending and receiving. */
    uint8_t consecIndex_;
    
    State state_ = STATE_NONE;
    
    std::mutex mutex_;

    void timeout();
    
    /* Thread used for sending consecutive frames */
    std::thread conThread_;
    
    std::condition_variable cv_;
    std::mutex cv_m_;
    
    /* Starts the timeout timer */
    void startTimer();
    /* Stops the timeout timer */
    void stopTimer();
    
    bool interruptTime_;
    int timeout_;
    void runTime(int timeout);
};

#endif // ISOTPINTERFACE_H
