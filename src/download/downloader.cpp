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

#include "downloader.h"
#include "definitions/definition.h"
#include "logger.h"
#include "protocols/udsprotocol.h"
#include "udsauthenticator.h"

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#ifdef WITH_SOCKETCAN
#include "protocols/socketcaninterface.h"
#endif

namespace download {

RMADownloader::RMADownloader(
    std::unique_ptr<uds::Protocol> &&uds, std::string key, std::size_t size)
    : uds_(std::move(uds)), key_(std::move(key)), totalSize_(size) {}



bool RMADownloader::update_progress() {
    notifyProgress((1.0f - (static_cast<float>(downloadSize_) / totalSize_)));
    return downloadSize_ > 0;
}



bool RMADownloader::download() {
    canceled_ = false;
    downloadOffset_ = 0;
    downloadSize_ = totalSize_;
    auth_.auth(key_, *uds_);

    do {
        std::vector<uint8_t> data = uds_->requestReadMemoryAddress(
            static_cast<uint32_t>(downloadOffset_), std::min<std::uint16_t>(static_cast<uint16_t>(downloadSize_), 0xFFE));

        if (data.empty()) {
            throw std::runtime_error("received 0 bytes in download packet");
        }

        downloadData_.insert(downloadData_.end(), data.begin(), data.end());
        downloadOffset_ += data.size();
        downloadSize_ -= data.size();
    } while (!canceled_ && update_progress());
    return !canceled_;
}



void RMADownloader::cancel() { canceled_ = true; }



std::pair<const uint8_t*, size_t> RMADownloader::data() {
    return std::make_pair(downloadData_.data(), downloadData_.size());
}

}
