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

#include "flasher.h"
#include "flashable.h"
#include "logger.h"
#include "protocols/caninterface.h"
#include "protocols/isotpprotocol.h"
#include "udsauthenticator.h"
#include "util.hpp"
#include "vehicle.h"

#include <cassert>
#include <sstream>
#include <utility>

#include <iostream>

namespace flash {

MazdaT1Flasher::MazdaT1Flasher(const PlatformLink &platform, const Options &options)
    : key_(options.key), uds_(platform.uds()) {
    if (!uds_) {
        throw std::runtime_error("UDS is unsupported with the selected datalink");
    }
}

bool MazdaT1Flasher::flash(const Flashable &flashable) {
    canceled_ = false;
    flash_ = &flashable;
    auth_.auth(*uds_, auth::Options{key_, 0x85});
    if (canceled_)
        return false;
    return do_erase();
}

void MazdaT1Flasher::cancel() { canceled_ = true; }



bool MazdaT1Flasher::do_erase() {
    std::array<uint8_t, 4> eraseRequest = {0xB1, 0x00, 0xB2, 0x00};
    uds::Packet _response;
    uds_->request(eraseRequest.data(), eraseRequest.size(), 0xF1, _response);
    if (canceled_) {
        Logger::warning("Canceled flash after erasing");
        return false;
    }
    return do_request_download();
}



bool MazdaT1Flasher::do_request_download() {
    // Send address...size
    std::array<uint8_t, 9> msg{};
    msg[0] = UDS_REQ_REQUESTDOWNLOAD;
    writeBE<int32_t>(flash_->offset(), msg.begin() + 1, msg.end());
    writeBE<int32_t>(flash_->data().size(), msg.begin() + 5, msg.end());

    uds::Packet _response;
    // Send download request
    uds_->request(msg.data(), msg.size(), UDS_RES_REQUESTDOWNLOAD, _response);

    if (canceled_) {
        Logger::warning("Canceled flash after erasure");
        return false;
    }

    // Start uploading
    sent_ = 0;
    left_ = flash_->data().size();
    return sendLoad();
}



bool MazdaT1Flasher::sendLoad() {
    while (left_ != 0) {
        size_t toSend = std::min<size_t>(left_, 0xFFE);
        std::vector<uint8_t> data;
        data.reserve(toSend + 1);
        data.emplace_back(UDS_REQ_TRANSFERDATA);
        data.insert(data.begin() + 1, flash_->data().begin() + sent_,
                    flash_->data().begin() + sent_ + toSend);

        sent_ += toSend;
        left_ -= toSend;

        uds::Packet _response;
        uds_->request(data.data(), data.size(), UDS_RES_TRANSFERDATA, _response);

        notifyProgress(static_cast<double>(sent_) / flash_->data().size());
        if (canceled_) {
            Logger::warning("Canceled flash during upload");
            return false;
        }
    }
    return true;
}

std::unique_ptr<Flasher> get_flasher(const std::string& id, const PlatformLink& link, const flash::Options& options)
{
    if (id == "mazdat1") {
        return std::make_unique<MazdaT1Flasher>(link, options);
    }
    return nullptr;
}


}
