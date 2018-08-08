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

class Uds23DownloadInterface : public DownloadInterface {
public:
  Uds23DownloadInterface(std::shared_ptr<isotp::Protocol> isotp,
                         std::string key, uint32_t size);
  void download() override;
  virtual gsl::span<const uint8_t> data();

  /* UdsAuthenticator Callback */
  void onAuthenticated(bool success, const std::string &error);

private:
  uds::Authenticator auth_;

  std::shared_ptr<uds::Protocol> uds_;

  std::string key_;

  /* Next memory location to be read from */
  size_t downloadOffset_{};
  /* Amount of data left to be transfered */
  size_t downloadSize_{};
  /* Total size to be transfered. Used for progress updates */
  size_t totalSize_;

  std::vector<uint8_t> downloadData_;

  bool checkError(uds::Error error);
  void do_download();
  bool update_progress();

};

Uds23DownloadInterface::Uds23DownloadInterface(std::shared_ptr<isotp::Protocol> isotp, std::string key, uint32_t size)
    : auth_(std::bind(&Uds23DownloadInterface::onAuthenticated, this,
                      std::placeholders::_1, std::placeholders::_2)),
      key_(std::move(key)), totalSize_(size) {
  uds_ = uds::Protocol::create(std::move(isotp));
}

std::unique_ptr<DownloadInterface> DownloadInterface::createM23(const std::shared_ptr<isotp::Protocol> &isotp, const std::string &key, uint32_t size)
{
    return std::make_unique<Uds23DownloadInterface>(isotp, key, size);
}

bool Uds23DownloadInterface::checkError(uds::Error error) {
  if (error != uds::Error::Success) {
    Logger::critical("Download error: " + uds::strError(error));
    notifyError(uds::strError(error));
    return false;
  }
  return true;
}

void Uds23DownloadInterface::onAuthenticated(bool success,
                                             const std::string &error) {
  if (!success) {
    notifyError(error);
    return;
  }
  do_download();
}

void Uds23DownloadInterface::do_download() {
  uds_->requestReadMemoryAddress(
      downloadOffset_, std::min<uint32_t>(downloadSize_, 0xFFE),
      [this](uds::Error error, gsl::span<const uint8_t> data) {
        if (!checkError(error)) {
          return;
        }

        if (data.empty()) {
          notifyError("received 0 bytes in download packet");
        }

        downloadData_.insert(downloadData_.end(), data.begin(), data.end());
        downloadOffset_ += data.size();
        downloadSize_ -= data.size();

        if (update_progress()) {
          do_download();
        }
      });
}

bool Uds23DownloadInterface::update_progress() {
  notifyProgress((1.0f - ((float)downloadSize_ / totalSize_)));
  if (downloadSize_ == 0) {
    Logger::info("Download complete");
    notifyComplete();
    return false;
  }

  return true;
}

void Uds23DownloadInterface::download() {
  downloadOffset_ = 0;
  downloadSize_ = totalSize_;
  auth_.auth(key_, uds_);
}

gsl::span<const uint8_t> Uds23DownloadInterface::data()
{
    return downloadData_;
}
