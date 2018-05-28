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
#include <iostream>
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
                         const std::shared_ptr<IsoTpInterface> &isotp,
                         const std::string &key, int size, const IsoTpOptions &options);
  void download() override;
  
  /* UDS Callback */
  void onRecv(const UdsResponse &message);

  /* UdsAuthenticator Callback */
  void onAuthenticated(bool success, const std::string &error);

private:
  enum State {
    STATE_AUTHENTICATING, // Waiting for auth success
    STATE_DOWNLOADING,
  };

  State state_;
  UdsAuthenticator auth_;
  
  IsoTpOptions options_;

  std::shared_ptr<IsoTpInterface> isotp_;
  
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
};

Uds23DownloadInterface::Uds23DownloadInterface(
    DownloadInterface::Callbacks *callbacks, const std::shared_ptr<IsoTpInterface> &isotp,
    const std::string &key, int size, const IsoTpOptions &options)
    : DownloadInterface(callbacks),
    options_(options), isotp_(isotp), auth_(std::bind(&Uds23DownloadInterface::onAuthenticated, this, std::placeholders::_1, std::placeholders::_2)),
      key_(key), totalSize_(size) {
  uds_ = UdsProtocol::create(isotp_, options_);
  
}

#ifdef WITH_SOCKETCAN
std::shared_ptr<DownloadInterface>
DownloadInterface::createSocketCan(DownloadInterface::Callbacks *callbacks,
                                   const std::string &device,
                                   DefinitionPtr definition) {
  assert(callbacks != nullptr);
  std::shared_ptr<SocketCanInterface> can;
  try {
      can = std::make_shared<SocketCanInterface>(device);
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
          callbacks, IsoTpInterface::get(can), definition->key(), definition->size(),
          IsoTpOptions(definition->serverId(), definition->serverId() + 8));
    default:
      callbacks->downloadError("CAN is not supported on this vehicle");
      break;
  }
  
  return nullptr;
}
#endif

void Uds23DownloadInterface::onAuthenticated(bool success, const std::string &error) {
  if (!success) {
    callbacks_->downloadError(QString::fromStdString(error));
    return;
  }
  state_ = STATE_DOWNLOADING;
  uds_->requestReadMemoryAddress(downloadOffset_,
                                 std::min<uint32_t>(downloadSize_, 0xFFE));
}

void Uds23DownloadInterface::download() {
  downloadOffset_ = 0;
  downloadSize_ = totalSize_; // 1 MiB
  // Start a UDS session using mode 0x87
  state_ = STATE_AUTHENTICATING;
  auth_.start(key_, isotp_, options_);
}

void Uds23DownloadInterface::onRecv(const UdsResponse &message) {
  if (!message.success()) {
    callbacks_->downloadError(QString::fromStdString(message.strError()));
  }
  
  if (message.id() == UDS_RES_NEGATIVE) {
    if (message.length() > 0) {
      onNegativeResponse(message[0]);
    } else {
      onNegativeResponse(0);
    }
    return;
  }

  switch (state_) {
  case STATE_DOWNLOADING:
    if (message.id() != UDS_RES_READMEM) {
      callbacks_->downloadError(
          "Unepected response. Expected ReadMemoryByAddress.");
      return;
    }

    if (message.length() == 0) {
      callbacks_->downloadError("Received 0 bytes in a download packet.");
      return;
    }

    downloadData_.insert(downloadData_.end(), message.message(),
                         message.message() + message.length());
    downloadOffset_ += message.length();
    downloadSize_ -= message.length();

    callbacks_->updateProgress((1.0f - ((float)downloadSize_ / totalSize_)) *
                               100.0f);

    if (downloadSize_ == 0) {
      // finished downloading
      callbacks_->onCompletion(downloadData_.data(), downloadData_.size());
    } else {
      uds_->requestReadMemoryAddress(downloadOffset_,
                                     std::min<uint32_t>(downloadSize_, 0xFFE));
    }

    break;
  default:
    // This should never happen
    assert(false);
  }
}

void Uds23DownloadInterface::onNegativeResponse(uint8_t code) {
  callbacks_->downloadError("Received negative UDS response: 0x" +
                            QString::number(code, 16));
}
