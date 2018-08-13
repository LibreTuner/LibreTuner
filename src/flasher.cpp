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
#include "protocols/caninterface.h"
#include "protocols/isotpprotocol.h"
#include "udsauthenticator.h"
#include "util.hpp"

#include <cassert>
#include <sstream>
#include <utility>

#include <iostream>



MazdaT1Flasher::MazdaT1Flasher(std::string key,
                               std::unique_ptr<uds::Protocol> &&uds)
    : key_(std::move(key)),
      uds_(std::move(uds)) {}

void MazdaT1Flasher::flash(FlashablePtr flashable) {
    flash_ = std::move(flashable);
    auth_.auth(key_, *uds_, 0x85);
    do_erase();
}



void MazdaT1Flasher::do_erase() {
    std::array<uint8_t, 4> eraseRequest = {0xB1, 0x00, 0xB2, 0x00};
    uds::Packet _response;
    uds_->request(eraseRequest, 0xF1, _response);
    do_request_download();
}



void MazdaT1Flasher::do_request_download() {
    // Send address...size
    std::array<uint8_t, 9> msg{};
    msg[0] = UDS_REQ_REQUESTDOWNLOAD;
    writeBE<int32_t>(flash_->offset(), gsl::make_span(msg).subspan(1));
    writeBE<int32_t>(flash_->size(), gsl::make_span(msg).subspan(5));

    uds::Packet _response;
    // Send download request
    uds_->request(msg, UDS_RES_REQUESTDOWNLOAD, _response);

    // Start uploading
    sent_ = 0;
    left_ = flash_->size();
    sendLoad();
}



void MazdaT1Flasher::sendLoad() {
    while (left_ != 0) {
        size_t toSend = std::min<size_t>(left_, 0xFFE);
        std::vector<uint8_t> data;
        data.reserve(toSend + 1);
        data.emplace_back(UDS_REQ_TRANSFERDATA);
        data.insert(data.begin() + 1, flash_->data() + sent_,
                  flash_->data() + sent_ + toSend);

        sent_ += toSend;
        left_ -= toSend;

        uds::Packet _response;
        uds_->request(data, UDS_RES_TRANSFERDATA, _response);

        notifyProgress(static_cast<double>(sent_) /
                         flash_->size());
    }
}
