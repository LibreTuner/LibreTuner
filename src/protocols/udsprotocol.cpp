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

#include "udsprotocol.h"
#include "caninterface.h"
#include "isotpinterface.h"

#include <cassert>


UdsResponse::UdsResponse(uint8_t id, const uint8_t* message, size_t length) : responseId_(id), message_(message, message + length)
{
}



void UdsResponse::setMessage(const uint8_t* message, size_t length)
{
    message_.assign(message, message + length);
}



class IsoTpUdsInterface : public UdsProtocol, public IsoTpInterface::Callbacks {
public:
    IsoTpUdsInterface(UdsProtocol::Callbacks *callbacks, CanInterfacePtr can, int srcId = 0x7E0, int dstId = 0x7E8);
    
    bool request(const uint8_t * message, size_t length) override;
    
    /* ISO-TP callbacks */
    void onCanError(CanInterface::CanError error, int err) override;
    void onError(IsoTpInterface::IsoTpError error) override;
    void onRecv(IsoTpMessagePtr message) override;
    void onTimeout() override;
    
private:
    IsoTpInterface isotp_;
};



bool IsoTpUdsInterface::request(const uint8_t* message, size_t length)
{
    auto res = isotp_.request(message, length);
    if (res != IsoTpInterface::ERR_SUCCESS)
    {
        callbacks_->onError(isotp_.strError(res));
        return false;
    }
    return true;
}



IsoTpUdsInterface::IsoTpUdsInterface(UdsProtocol::Callbacks* callbacks, std::shared_ptr<CanInterface> can, int srcId, int dstId) : UdsProtocol(callbacks), isotp_(this, can, srcId, dstId)
{
}



void IsoTpUdsInterface::onCanError(CanInterface::CanError error, int err)
{
    callbacks_->onError(CanInterface::strError(error, err));
}



void IsoTpUdsInterface::onError(IsoTpInterface::IsoTpError error)
{
    callbacks_->onError(isotp_.strError(error));
}



void IsoTpUdsInterface::onRecv(IsoTpMessagePtr message)
{
    if (message->length() == 0)
    {
        // Silently fail
        // callbacks_->onError("received message with size 0");
        return;
    }
        
    UdsResponse res(message->message()[0], message->message() + 1, message->length() - 1);
    
    callbacks_->onRecv(res);
}



void IsoTpUdsInterface::onTimeout()
{
    callbacks_->onTimeout();
}





UdsProtocol::UdsProtocol(UdsProtocol::Callbacks* callbacks) : callbacks_(callbacks)
{
}



std::shared_ptr<UdsProtocol> UdsProtocol::create(UdsProtocol::Callbacks* callbacks, std::shared_ptr<CanInterface> can, int srcId, int dstId)
{
    assert(callbacks != nullptr);
    return std::make_shared<IsoTpUdsInterface>(callbacks, can);
}



bool UdsProtocol::requestSession(uint8_t type)
{
    uint8_t req[] = {UDS_REQ_SESSION, type};
    return request(req, sizeof(req));
}



bool UdsProtocol::requestSecuritySeed()
{
    uint8_t req[] = {UDS_REQ_SECURITY, 1};
    return request(req, sizeof(req));
}



bool UdsProtocol::requestSecurityKey(const uint8_t* key, uint8_t length)
{
    std::vector<uint8_t> req(length + 2);
    req[0] = UDS_REQ_SECURITY;
    req[1] = 2;
    memcpy(req.data() + 2, key, length);
    return request(req.data(), length + 2);
}





bool UdsProtocol::requestReadMemoryAddress(const uint32_t address, uint16_t length)
{
    uint8_t req[7];
    req[0] = UDS_REQ_READMEM;
    
    req[1] = (address & 0xFF000000) >> 24;
    req[2] = (address & 0xFF0000) >> 16;
    req[3] = (address & 0xFF00) >> 8;
    req[4] = address & 0xFF;
    
    req[5] = length >> 8;
    req[6] = length & 0xFF;
    
    return request(req, sizeof(req));
}
