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
#include "protocols/isotpinterface.h"
#include "udsauthenticator.h"
#include "util.hpp"

#include <cassert>
#include <sstream>

Flasher::Flasher(Flasher::Callbacks *callbacks) : callbacks_(callbacks) {
  assert(callbacks != nullptr);
}

class MazdaT1Flasher : public Flasher {
public:
  MazdaT1Flasher(Flasher::Callbacks *callbacks, const std::string &key, const std::shared_ptr<IsoTpInterface> &isotp,
                 const IsoTpOptions &options);

  void flash(FlashablePtr flashable) override;

  enum State {
    STATE_NONE,
    STATE_AUTHENTICATING,
    STATE_ERASING,
    STATE_DOWN_REQUEST, // Waiting for response from RequestDownload
    STATE_UPLOADING,
  };

  /* UdsAuthenticator callbacks */
  void onAuthenticated(bool success, const std::string &error);

  /* ISO-TP Callbacks */
  void onRecv(const IsoTpInterface::Response &message);

private:
  IsoTpOptions options_;
  std::shared_ptr<IsoTpInterface> isotp_;
  FlashablePtr flash_;
  UdsAuthenticator auth_;
  std::string key_;

  size_t left_, sent_;

  State state_;

  void onFail(const std::string &error);
  void request(const uint8_t *data, size_t length);
  void sendLoad();
};

MazdaT1Flasher::MazdaT1Flasher(Flasher::Callbacks *callbacks,
                               const std::string &key, const std::shared_ptr<IsoTpInterface> &isotp,
                               const IsoTpOptions &options)
: Flasher(callbacks), options_(options), isotp_(isotp), auth_(std::bind(&MazdaT1Flasher::onAuthenticated, this, std::placeholders::_1, std::placeholders::_2)) {
  options_.setTimeout(std::chrono::milliseconds(2000));
}

void MazdaT1Flasher::flash(FlashablePtr flashable) {
  state_ = STATE_AUTHENTICATING;
  auth_.start(key_, isotp_, options_);
}

void MazdaT1Flasher::onFail(const std::string &error) {
  state_ = STATE_NONE;
  callbacks_->onError(error);
}

void MazdaT1Flasher::request(const uint8_t *data, size_t length) {
  isotp_->request(data, length, options_).then([this](boost::future<IsoTpInterface::Response> f) {
    onRecv(f.get());
  });
}

void MazdaT1Flasher::onAuthenticated(bool success, const std::string &error) {
  assert(state_ == STATE_AUTHENTICATING);
  
  if (!success) {
    onFail(error);
    return;
  }

  // Start erasing
  state_ = STATE_ERASING;
  uint8_t eraseRequest[] = {0xB1, 0x00, 0xB2, 0x00};
  request(eraseRequest, sizeof(eraseRequest));
}

void MazdaT1Flasher::sendLoad() {
  size_t toSend = std::min<size_t>(left_, 0xFFE);
  std::vector<uint8_t> data;
  data.reserve(toSend + 1);
  data[0] = UDS_REQ_TRANSFERDATA;
  data.insert(data.begin() + 1, flash_->data() + sent_,
              flash_->data() + sent_ + toSend);
  request(data.data(), data.size());
  sent_ += toSend;
  left_ -= toSend;
  callbacks_->onProgress(static_cast<double>(sent_) / flash_->size());
  if (left_ == 0) {
    state_ = STATE_NONE;
    callbacks_->onCompletion();
  }
}

void MazdaT1Flasher::onRecv(const IsoTpInterface::Response &response) {
  if (!response.success()) {
    onFail(response.errorString());
    return;
  }
  
  IsoTpMessage &message = response.message();
  
  if (message.length() == 0) {
    onFail("Received ISO-TP message of length 0");
    return;
  }

  int id = message.message()[0];
  if (id == UDS_RES_NEGATIVE) {
    int res = 0;
    if (message.length() > 1) {
      res = message.message()[1];
    }
    std::stringstream ss;
    ss << "Received negative UDS response: 0x" << std::hex << res;
    onFail(ss.str());
    return;
  }

  switch (state_) {
  case STATE_ERASING: {
    if (id != 0xF1) {
      std::stringstream ss;
      ss << "Unexpected response 0x" << std::hex << id
         << ". Expected 0xF1 (erase)";
      onFail(ss.str());
      return;
    }

    // Send address...size
    uint8_t msg[9];
    msg[0] = UDS_REQ_REQUESTDOWNLOAD;
    writeBEInt32(flash_->offset(), msg + 1);
    writeBEInt32(flash_->size(), msg + 5);
    // Send download request
    state_ = STATE_DOWN_REQUEST;
    request(msg, sizeof(msg));

    break;
  }
  case STATE_DOWN_REQUEST:
    if (id != UDS_RES_REQUESTDOWNLOAD) {
      std::stringstream ss;
      ss << "Unexpected response 0x" << std::hex << id
         << ". Expected RequestDownload";
      onFail(ss.str());
      return;
    }

    // Start uploading
    sent_ = 0;
    left_ = flash_->size();
    state_ = STATE_UPLOADING;
    sendLoad();
    break;
  case STATE_UPLOADING:
    if (id != UDS_RES_TRANSFERDATA) {
      std::stringstream ss;
      ss << "Unexpected response 0x" << std::hex << id
         << ". Expected TransferData";
      onFail(ss.str());
      return;
    }

    sendLoad();
    break;
  default:
    // This should never happen
    assert(false);
  }
}

FlasherPtr Flasher::createT1(Flasher::Callbacks* callbacks, const std::string& key, const std::shared_ptr<IsoTpInterface> &isotp, const IsoTpOptions& options) {
  return std::make_shared<MazdaT1Flasher>(callbacks, key, isotp, options);
}

