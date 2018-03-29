#include "isotpinterface.h"

#include <cassert>
#include <chrono>

#include <iostream>


IsoTpMessage::IsoTpMessage(int id) : id_(id)
{
    
}



void IsoTpMessage::append(const uint8_t* message, size_t length)
{
    message_.insert(message_.end(), message, message + length);
}



IsoTpInterface::IsoTpInterface(Callbacks *callbacks, std::shared_ptr<CanInterface> can, int srcId, int dstId) : callbacks_(callbacks), canInterface_(can), srcId_(srcId), dstId_(dstId)
{
    assert(can != nullptr);
    assert(callbacks != nullptr);
    can->addCallbacks(this);
}



void IsoTpInterface::onError(CanInterface::CanError error, int err)
{
    callbacks_->onCanError(error, err);
}



// TODO: this function should be cleaned up in the future
void IsoTpInterface::onRecv(const CanMessage& message)
{
    std::unique_lock<std::mutex> lk(mutex_);
    if (message.id() != dstId_ || state_ == STATE_NONE)
    {
        if (message.id() == dstId_)
        {
            std::cout << "Lost message" << std::endl;
        }
        return;
    }
    
    if (message.length() == 0)
    {
        return;
    }
    
    uint8_t type = message[0] & 0xF0;
    
    switch(state_)
    {
        case STATE_RESP:
        {
            if (type == 0x0)
            {
                stopTimer();
                if (message.length() < (message[0] & 0x0F) + 1)
                {
                    // The CAN frame is not long enough
                    state_ = STATE_NONE;
                    recvMsg_.reset();
                    callbacks_->onError(ERR_SIZE);
                    
                    return;
                }
                // Single frame
                recvMsg_->append(&message[1], message[0] & 0x0F);
                state_ = STATE_NONE;
                
                std::shared_ptr<IsoTpMessage> msg(recvMsg_);
                recvMsg_.reset();
                
                lk.unlock();
                callbacks_->onRecv(msg);
                lk.lock();
                
                
                break;
            }
            else if (type == 0x10)
            {
                stopTimer();
                if (message.length() < 2)
                {
                    // The CAN frame is not long enough
                    state_ = STATE_NONE;
                    recvMsg_.reset();
                    callbacks_->onError(ERR_SIZE);
                    
                    return;
                }
                // First frame
                recvRemaining_ = ((message[0] & 0x0F) << 8) | message[1];
                if (recvRemaining_ < 7)
                {
                    // The message should have been sent in a single frame
                    if (message.length() < recvRemaining_ + 2)
                    {
                        // The CAN frame is not long enough
                        state_ = STATE_NONE;
                        recvMsg_.reset();
                        callbacks_->onError(ERR_SIZE);
                        
                        return;
                    }
                    
                    recvMsg_->append(&message[2], recvRemaining_);
                    state_ = STATE_NONE;
                    
                    std::shared_ptr<IsoTpMessage> msg(recvMsg_);
                    recvMsg_.reset();
                
                    lk.unlock();
                    callbacks_->onRecv(msg);
                    lk.lock();
                    
                    break;
                }
                
                if (message.length() < 8)
                {
                    // The CAN frame is not long enough
                    state_ = STATE_NONE;
                    recvMsg_.reset();
                    callbacks_->onError(ERR_SIZE);
                    
                    return;
                }
                
                recvMsg_->append(&message[2], 6);
                recvRemaining_ -= 6;
                
                // Send a flow control frame
                CanMessage resp;
                resp.id_ = srcId_;
                resp.messageLength_ = 8;
                resp[0] = 0x30; // FC flag = 0 (clear to send)
                resp[1] = 0; // Block size
                resp[2] = 0; // Time to delay between frames
                if (!canInterface_->send(resp))
                {
                    // Send failed; abort
                    state_ = STATE_NONE;
                    recvMsg_.reset();
                    callbacks_->onError(ERR_CAN);
                    
                    return;
                }
                
                startTimer();
                consecIndex_ = 1;
                state_ = STATE_CONSEC;
            }
            break;
        }
        case STATE_FLOW:
        {
            if (type != 0x30)
            {
                break;
            }
            
            stopTimer();
            
            if (message.length() < 3)
            {
                // The CAN frame is not long enough
                state_ = STATE_NONE;
                recvMsg_.reset();
                callbacks_->onError(ERR_SIZE);
                
                return;
            }
            
            uint8_t fcFlag = message[0] & 0x0F;
            uint8_t blockSize = message[1];
            uint8_t sepTime = message[2]; // Separation time
            consecIndex_ = 1;
            
            conThread_ = std::thread([this, fcFlag, blockSize, sepTime] (){
                uint8_t blocksRemaining = blockSize;
                std::unique_lock<std::mutex> lk(mutex_);
                
                CanMessage m;
                m.id_ = dstId_;
                m.messageLength_ = 8;
                
                while (msgPtr_ != msg_.end())
                {
                    if (sepTime > 0)
                    {
                        lk.unlock();
                        if (sepTime <= 127)
                        {
                            // milliseconds
                            std::this_thread::sleep_for(std::chrono::milliseconds(sepTime));
                        }
                        else if (sepTime >= 0xF1 && sepTime <= 0xF9)
                        {
                            std::this_thread::sleep_for(std::chrono::microseconds((sepTime - 0xF0) * 100));
                        }
                        lk.lock();
                    }
                    
                    m[0] = 0x20 | consecIndex_;
                    uint8_t toWrite = std::min<int>(7, msg_.end() - msgPtr_);
                    std::copy(msgPtr_, msgPtr_ + toWrite, &m[1]);
                    msgPtr_ += toWrite;
                    
                    if (!canInterface_->send(m))
                    {
                        // Send failed; abort
                        state_ = STATE_NONE;
                        recvMsg_.reset();
                        callbacks_->onError(ERR_CAN);
                        
                        break;
                    }
                    
                    if (blocksRemaining > 0)
                    {
                        if (blocksRemaining == 1)
                        {
                            state_ = STATE_FLOW;
                            startTimer();
                            break;
                        }
                        
                        --blocksRemaining;
                    }
                }
            });
            
            break;
        }
        case STATE_CONSEC:
        {
            if (type != 0x20)
            {
                break;
            }
            
            stopTimer();
            
            if ((message[0] & 0x0F) != consecIndex_)
            {
                // Incorrect consecutive consecutive index
                state_ = STATE_NONE;
                recvMsg_.reset();
                callbacks_->onError(ERR_CONSEC);
                
                return;
            }
            
            uint8_t toRead = std::min<int>(7, recvRemaining_);
            
            if (message.length() < toRead + 1)
            {
                // The CAN frame is not long enough
                state_ = STATE_NONE;
                recvMsg_.reset();
                callbacks_->onError(ERR_SIZE);
                
                return;
            }
            
            recvMsg_->append(&message[1], toRead);
            recvRemaining_ -= toRead;
            if (recvRemaining_ == 0)
            {
                state_ = STATE_NONE;
                
                std::shared_ptr<IsoTpMessage> msg(recvMsg_);
                recvMsg_.reset();
                
                lk.unlock();
                callbacks_->onRecv(msg);
                lk.lock();
                
                return;
            }
            
            if (++consecIndex_ == 16)
            {
                consecIndex_ = 0;
            }
            
            startTimer();
            
            break;
        }
        default:
            break;
    }
}



void IsoTpInterface::runTime(int delay)
{
    std::unique_lock<std::mutex> lk(cv_m_);
    if (!cv_.wait_for(lk, std::chrono::milliseconds(delay), [this]() {return interruptTime_; }))
    {
        timeout();
    }
}



void IsoTpInterface::startTimer()
{
    stopTimer();
    
    interruptTime_ = false;
    conThread_ = std::thread(&IsoTpInterface::runTime, this, timeout_);
}



void IsoTpInterface::stopTimer()
{
    if (conThread_.joinable())
    {
        {
            std::lock_guard<std::mutex> lk(cv_m_);
            interruptTime_ = true;
        }
        
        cv_.notify_all();
        conThread_.join();
    }
}



IsoTpInterface::IsoTpError IsoTpInterface::request(const uint8_t* message, size_t length, int timeout)
{
    assert(length <= 4095);
    std::lock_guard<std::mutex> lk(mutex_);
    
    timeout_ = timeout;
    
    if (state_ != STATE_NONE)
    {
        return ERR_BUSY;
    }
    
    if (length < 8)
    {
        // Send the request in a single frame
        CanMessage msg;
        msg.id_ = srcId_;
        msg.messageLength_ = 8;
        msg.message_[0] = length;
        memcpy(&msg.message_[1], message, length);
        
        if (!canInterface_->send(msg))
        {
            stopTimer();
            state_ = STATE_NONE;
            recvMsg_.reset();
            return ERR_CAN;
        }
        
        recvMsg_ = std::make_shared<IsoTpMessage>(dstId_);
        state_ = STATE_RESP;
        startTimer();
    }
    else
    {
        // Send a first frame
        CanMessage msg;
        msg.id_ = srcId_;
        msg.messageLength_ = 8;
        msg.message_[0] = 0x20 | ((length & 0xF00) >> 8);
        msg.message_[1] = length | 0xFF;
        memcpy(&msg.message_[2], message, 6);
        
        if (!canInterface_->send(msg))
        {
            return ERR_CAN;
        }
        
        msg_.assign(message + 6, message + length - 6);
        msgPtr_ = msg_.begin();
        
        state_ = STATE_FLOW;
        recvMsg_ = std::make_shared<IsoTpMessage>(dstId_);
        startTimer();
    }
    
    return ERR_SUCCESS;
}



void IsoTpInterface::timeout()
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        state_ = STATE_NONE;
        recvMsg_.reset();
    }
    callbacks_->onTimeout();
}



std::string IsoTpInterface::strError(IsoTpInterface::IsoTpError error)
{
    switch(error)
    {
        case ERR_BUSY:
            return "a request is already active on the ISO-TP interface";
        case ERR_CAN:
            return "an error occured on the CAN interface";
        case ERR_CONSEC:
            return "a received consecutive frame index was incorrect";
        case ERR_SIZE:
            return "a received CAN frame was too short";
        case ERR_SUCCESS:
            return "success";
        default:
            break;
    }
    
    return "unknown. You should never see this. If you do, please submit a pull request";
}



IsoTpInterface::~IsoTpInterface()
{
    stopTimer();
    if (canInterface_)
    {
        canInterface_->removeCallbacks(this);
    }
}
