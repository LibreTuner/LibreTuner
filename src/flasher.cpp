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

Flasher::Flasher(Flasher::Callbacks *callbacks) : callbacks_(callbacks) {
  assert(callbacks != nullptr);
}

class MazdaT1Flasher : public Flasher {
public:
  MazdaT1Flasher(Flasher::Callbacks *callbacks, const std::string &key, std::shared_ptr<isotp::Protocol> isotp);

  void flash(FlashablePtr flashable) override;

  /* UdsAuthenticator callbacks */
  void onAuthenticated(bool success, const std::string &error);

private:
  std::shared_ptr<uds::Protocol> uds_;
  FlashablePtr flash_;
  uds::Authenticator auth_;
  std::string key_;

  size_t left_{}, sent_{};

  void onFail(const std::string &error);
  void sendLoad();
  void do_erase();
  void do_request_download();
};

MazdaT1Flasher::MazdaT1Flasher(Flasher::Callbacks *callbacks,
                               const std::string &key, std::shared_ptr<isotp::Protocol> isotp)
: Flasher(callbacks), key_(key), uds_(uds::Protocol::create(std::move(isotp))), auth_(std::bind(&MazdaT1Flasher::onAuthenticated, this, std::placeholders::_1, std::placeholders::_2)) {

}

void MazdaT1Flasher::flash(FlashablePtr flashable) {
  flash_ = flashable;
  auth_.auth(key_, uds_, 0x85);
}

void MazdaT1Flasher::onFail(const std::string &error) {
  callbacks_->onError(error);
}

void MazdaT1Flasher::onAuthenticated(bool success, const std::string &error) {
  if (!success) {
    onFail(error);
    return;
  }
  do_erase();
}

void MazdaT1Flasher::do_erase() {
  std::array<uint8_t, 4> eraseRequest = {0xB1, 0x00, 0xB2, 0x00};
  uds_->request(eraseRequest, 0xF1, [this](uds::Error error, const uds::Packet &packet) {
    if (error != uds::Error::Success) {
      onFail(uds::strError(error));
      return;
    }

    do_request_download();
  });
}

void MazdaT1Flasher::do_request_download() {
  // Send address...size
  std::array<uint8_t, 9> msg;
  msg[0] = UDS_REQ_REQUESTDOWNLOAD;
  writeBE<int32_t>(flash_->offset(), gsl::make_span(msg).subspan(1));
  writeBE<int32_t>(flash_->size(), gsl::make_span(msg).subspan(5));
  // Send download request
  uds_->request(msg, UDS_RES_REQUESTDOWNLOAD, [this](uds::Error error, const uds::Packet &packet) {
    // Start uploading
    sent_ = 0;
    left_ = flash_->size();
    sendLoad();
  });
}

void MazdaT1Flasher::sendLoad() {
  size_t toSend = std::min<size_t>(left_, 0xFFE);
  std::vector<uint8_t> data;
  data.reserve(toSend + 1);
  data.emplace_back(UDS_REQ_TRANSFERDATA);
  data.insert(data.begin() + 1, flash_->data() + sent_,
              flash_->data() + sent_ + toSend);

  sent_ += toSend;
  left_ -= toSend;

  uds_->request(data, UDS_RES_TRANSFERDATA, [this](uds::Error error, const uds::Packet &packet) {
    if (error != uds::Error::Success) {
      onFail(uds::strError(error));
    }

    callbacks_->onProgress(static_cast<double>(sent_) / flash_->size());
    if (left_ == 0) {
      callbacks_->onCompletion();
    } else {
      sendLoad();
    }
  });
}

FlasherPtr Flasher::createT1(Flasher::Callbacks* callbacks, const std::string& key, std::shared_ptr<isotp::Protocol> isotp) {
  return std::make_shared<MazdaT1Flasher>(callbacks, key, isotp);
}

