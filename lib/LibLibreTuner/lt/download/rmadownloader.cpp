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

#include "rmadownloader.h"
#include "auth/udsauthenticator.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

namespace lt {
namespace download {

RMADownloader::RMADownloader(network::UdsPtr &&uds, Options &&options)
    : uds_(std::move(uds)), authOptions_(std::move(options.auth)),
      totalSize_(options.size) {
    if (!uds_) {
        throw std::runtime_error(
            "UDS is unsupported with the selected datalink");
    }
}

bool RMADownloader::update_progress() {
    notifyProgress((1.0f - (static_cast<float>(downloadSize_) / totalSize_)));
    return downloadSize_ > 0;
}

bool RMADownloader::download() {
    canceled_ = false;
    downloadOffset_ = 0;
    downloadSize_ = totalSize_;

    // Authenticate
    auth::UdsAuthenticator auth(*uds_, authOptions_);
    auth.auth();

    do {
        size_t to_download =
            std::min<std::size_t>(static_cast<size_t>(downloadSize_), 0xFFE);
        std::vector<uint8_t> data = uds_->requestReadMemoryAddress(
            static_cast<uint32_t>(downloadOffset_),
            static_cast<uint16_t>(to_download));

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

std::pair<const uint8_t *, size_t> RMADownloader::data() {
    return std::make_pair(downloadData_.data(), downloadData_.size());
}

} // namespace download
} // namespace lt