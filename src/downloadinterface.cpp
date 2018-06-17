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

#include <algorithm>
#include <cassert>
#include <vector>

#ifdef WITH_SOCKETCAN
#include "protocols/socketcaninterface.h"
#endif

DownloadInterface::DownloadInterface(DownloadInterface::Callbacks *callbacks)
    : callbacks_(callbacks) {
  assert(callbacks_ != nullptr);
}

class Uds23DownloadInterface : public DownloadInterface {
public:
  Uds23DownloadInterface(DownloadInterface::Callbacks *callbacks,
                         std::shared_ptr<isotp::Protocol> isotp,
                         const std::string &key, int size);
  void download() override;

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

Uds23DownloadInterface::Uds23DownloadInterface(
    DownloadInterface::Callbacks *callbacks,
    std::shared_ptr<isotp::Protocol> isotp, const std::string &key, int size)
    : DownloadInterface(callbacks),
      auth_(std::bind(&Uds23DownloadInterface::onAuthenticated, this,
                      std::placeholders::_1, std::placeholders::_2)),
      key_(key), totalSize_(size) {
  uds_ = uds::Protocol::create(std::move(isotp));
}

#ifdef WITH_SOCKETCAN
std::shared_ptr<DownloadInterface>
DownloadInterface::createSocketCan(DownloadInterface::Callbacks *callbacks,
                                   const std::string &device,
                                   DefinitionPtr definition) {
  assert(callbacks != nullptr);
  std::shared_ptr<SocketCanInterface> can;
  try {
    can = SocketCanInterface::create(device);
    can->start();
  } catch (std::exception &e) {
    callbacks->downloadError("Could not initialize SocketCAN device \"" +
                             QString::fromStdString(device) +
                             "\": " + QString(e.what()));
    return nullptr;
  }

  switch (definition->downloadMode()) {
  case DM_MAZDA23:
    return std::make_shared<Uds23DownloadInterface>(
        callbacks,
        std::make_shared<isotp::Protocol>(
            can,
            isotp::Options{definition->serverId(), definition->serverId() + 8,
                           std::chrono::milliseconds(100)}),
        definition->key(), definition->size());
  default:
    callbacks->downloadError("CAN is not supported on this vehicle");
    break;
  }

  return nullptr;
}
#endif

bool Uds23DownloadInterface::checkError(uds::Error error) {
  if (error != uds::Error::Success) {
    callbacks_->downloadError(QString::fromStdString(uds::strError(error)));
    return false;
  }
  return true;
}

void Uds23DownloadInterface::onAuthenticated(bool success,
                                             const std::string &error) {
  if (!success) {
    callbacks_->downloadError(QString::fromStdString(error));
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
          callbacks_->downloadError("received 0 bytes in download packet");
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
  callbacks_->updateProgress((1.0f - ((float)downloadSize_ / totalSize_)));
  if (downloadSize_ == 0) {
    callbacks_->onCompletion(downloadData_);
    return false;
  }
  return true;
}

void Uds23DownloadInterface::download() {
  downloadOffset_ = 0;
  downloadSize_ = totalSize_;
  auth_.auth(key_, uds_);
}