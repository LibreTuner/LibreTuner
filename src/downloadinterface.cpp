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

#include "downloadinterface.h"
#include "protocols/udsprotocol.h"
#include "definitions/definition.h"

#include <cassert>
#include <vector>
#include <iostream>
#include <algorithm>

#ifdef WITH_SOCKETCAN
#include "protocols/socketcaninterface.h"
#endif

DownloadInterface::DownloadInterface(DownloadInterface::Callbacks* callbacks) : callbacks_(callbacks)
{
    assert(callbacks_ != nullptr);
}



class Uds23DownloadInterface : public DownloadInterface, public UdsProtocol::Callbacks
{
public:
    Uds23DownloadInterface(DownloadInterface::Callbacks *callbacks, std::shared_ptr<CanInterface> can, const std::string &key, int size, int srcId = 0x7E0, int dstId = 0x7E8);
    
    void download() override;
    
    void onError(const std::string &error) override;
    
    void onRecv(const UdsResponse &message) override;
    
    void onTimeout() override;
    
private:
    enum State {
        STATE_SESSION, // Waiting for response from session control
        STATE_SECURITY_REQUEST, // Waiting for response from security access requestSeed
        STATE_SECURITY_KEY, // Waiting for response from security access sendKey
        STATE_DOWNLOADING,
    };
    
    State state_;
    std::shared_ptr<UdsProtocol> uds_;
    
    std::string key_;
    
    /* Next memory location to be read from */
    size_t downloadOffset_;
    /* Amount of data left to be transfered */
    size_t downloadSize_;
    /* Total size to be transfered. Used for progress updates */
    size_t totalSize_;
    
    std::vector<uint8_t> downloadData_;
    
    void onNegativeResponse(uint8_t code);
    
    uint32_t generateKey(uint32_t parameter, const uint8_t *seed, size_t len);
};



Uds23DownloadInterface::Uds23DownloadInterface(DownloadInterface::Callbacks *callbacks, std::shared_ptr<CanInterface> can, const std::string &key, int size, int srcId, int dstId) : DownloadInterface(callbacks), uds_(UdsProtocol::create(this, can, srcId, dstId)), key_(key), totalSize_(size)
{
    
}



#ifdef WITH_SOCKETCAN
std::shared_ptr<DownloadInterface> DownloadInterface::createSocketCan(DownloadInterface::Callbacks* callbacks, const std::string& device, DefinitionPtr definition)
{
    assert(callbacks != nullptr);
    std::shared_ptr<SocketCanInterface> can = std::make_shared<SocketCanInterface>(nullptr, device);
    if (!can->valid())
    {
        callbacks->downloadError("Could not initialize SocketCAN device \"" + QString::fromStdString(device) + "\": " + QString::fromStdString(can->strError()));
        return nullptr;
    }
    
    can->start();
    
    switch (definition->downloadMode())
    {
    case DM_MAZDA23:
        // Mazdaspeed 6 ROM size is 1 MiB
        return std::make_shared<Uds23DownloadInterface>(callbacks, can, definition->key(), 1024 * 1024, definition->serverId(), definition->serverId() + 8);
    default:
        callbacks->downloadError("CAN is not supported on this vehicle");
        break;
    }
    
    
    return nullptr;
}
#endif


void Uds23DownloadInterface::download()
{
    downloadOffset_ = 0;
    downloadSize_ = totalSize_; // 1 MiB
    // Start a UDS session using mode 0x87
    state_ = STATE_SESSION;
    if (!uds_->requestSession(0x87))
    {
        // onError should have been called
        return;
    }
}



void Uds23DownloadInterface::onError(const std::string& error)
{
    callbacks_->downloadError(QString::fromStdString(error));
}



uint32_t Uds23DownloadInterface::generateKey(uint32_t parameter, const uint8_t *seed, size_t len)
{
    std::vector<uint8_t> nseed(seed, seed + len);
    nseed.insert(nseed.end(), key_.begin(), key_.end());
    
    for (uint8_t c : nseed)
    {
        for (int r = 8; r > 0; --r)
        {
            uint8_t s = (c & 1) ^ (parameter & 1);
            uint32_t m = 0;
            if (s != 0)
            {
                parameter |= 0x1000000;
                m = 0x109028;
            }
            
            c >>= 1;
            parameter >>= 1;
            uint32_t p3 = parameter & 0xFFEF6FD7;
            parameter ^= m;
            parameter &= 0x109028;
            
            parameter |= p3;
            
            parameter &= 0xFFFFFF;
        }
    }
    
    uint32_t res = (parameter >> 4) & 0xFF;
    res |= (((parameter >> 20) & 0xFF) + ((parameter >> 8) & 0xF0)) << 8;
    res |= (((parameter << 4) & 0xFF) + ((parameter >> 16) & 0xF)) << 16;
    
    return res;
}



void Uds23DownloadInterface::onRecv(const UdsResponse &message)
{
    if (message.id() == UDS_RES_NEGATIVE)
    {
        if (message.length() > 0)
        {
            onNegativeResponse(message[0]);
        }
        else
        {
            onNegativeResponse(0);
        }
        return;
    }
    
    switch(state_)
    {
        case STATE_SESSION:
            if (message.id() != UDS_RES_SESSION)
            {
                callbacks_->downloadError("Unepected response. Expected session.");
                return;
            }
            
            // Request security seed
            state_ = STATE_SECURITY_REQUEST;
            uds_->requestSecuritySeed();
            
            break;
        case STATE_SECURITY_REQUEST:
        {
            if (message.id() != UDS_RES_SECURITY)
            {
                callbacks_->downloadError("Unepected response. Expected security.");
                return;
            }
            
            // TODO: Change to actual length
            if (message.length() < 2)
            {
                callbacks_->downloadError("Unexpected message length in security request response");
                return;
            }
            
            // Generate key from seed
            uint32_t key = generateKey(0xC541A9, reinterpret_cast<const uint8_t*>(message.message() + 1), message.length() - 1);
            
            uint8_t kData[3];
            kData[0] = key & 0xFF;
            kData[1] = (key & 0xFF00) >> 8;
            kData[2] = (key & 0xFF0000) >> 16;
            
            // Send key
            state_ = STATE_SECURITY_KEY;
            uds_->requestSecurityKey(kData, sizeof(kData));
            
            break;
        }
        case STATE_SECURITY_KEY:
            if (message.id() != UDS_RES_SECURITY)
            {
                callbacks_->downloadError("Unepected response. Expected security.");
                return;
            }
            
            // Begin downloading
            state_ = STATE_DOWNLOADING;
            uds_->requestReadMemoryAddress(downloadOffset_, std::min<uint32_t>(downloadSize_, 0xFFE));
            
            break;
        case STATE_DOWNLOADING:
            if (message.id() != UDS_RES_READMEM)
            {
                callbacks_->downloadError("Unepected response. Expected ReadMemoryByAddress.");
                return;
            }
            
            if (message.length() == 0)
            {
                callbacks_->downloadError("Received 0 bytes in a download packet.");
                return;
            }
            
            downloadData_.insert(downloadData_.end(), message.message(), message.message() + message.length());
            downloadOffset_ += message.length();
            downloadSize_ -= message.length();
            
            callbacks_->updateProgress((1.0f - ((float)downloadSize_ / totalSize_)) * 100.0f);
            
            if (downloadSize_ == 0)
            {
                // finished downloading
                callbacks_->onCompletion(downloadData_.data(), downloadData_.size());
            }
            else
            {
                uds_->requestReadMemoryAddress(downloadOffset_, std::min<uint32_t>(downloadSize_, 0xFFE));
            }
            
            break;
    }
}



void Uds23DownloadInterface::onNegativeResponse(uint8_t code)
{
    callbacks_->downloadError("Received negative UDS response: 0x" + QString::number(code, 16));
}



void Uds23DownloadInterface::onTimeout()
{
    callbacks_->downloadError("an ISO-TP request timed out");
}
