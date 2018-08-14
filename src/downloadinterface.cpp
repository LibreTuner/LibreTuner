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
#include "definitions/definition.h"
#include "protocols/udsprotocol.h"
#include "udsauthenticator.h"
#include "logger.h"

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#ifdef WITH_SOCKETCAN
#include "protocols/socketcaninterface.h"
#endif

Uds23DownloadInterface::Uds23DownloadInterface(std::unique_ptr<uds::Protocol> &&uds, std::string key, uint32_t size)
    : key_(std::move(key)), totalSize_(size), uds_(std::move(uds)) {
}



bool Uds23DownloadInterface::update_progress() {
  notifyProgress((1.0f - ((float)downloadSize_ / totalSize_)));
  return downloadSize_ > 0;
}



bool Uds23DownloadInterface::download() {
  canceled_ = false;
  downloadOffset_ = 0;
  downloadSize_ = totalSize_;
  auth_.auth(key_, *uds_);

  do {
      std::vector<uint8_t> data = uds_->requestReadMemoryAddress(downloadOffset_, std::min<uint32_t>(downloadSize_, 0xFFE));

      if (data.empty()) {
          throw std::runtime_error("received 0 bytes in download packet");
      }

      downloadData_.insert(downloadData_.end(), data.begin(), data.end());
      downloadOffset_ += data.size();
      downloadSize_ -= data.size();
  } while (!canceled_ && update_progress());
  return !canceled_;
}



void Uds23DownloadInterface::cancel()
{
    canceled_ = true;
}



gsl::span<const uint8_t> Uds23DownloadInterface::data()
{
    return downloadData_;
}
