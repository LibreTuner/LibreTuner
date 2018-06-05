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
#include <protocols/isotpinterface.h>

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
  std::shared_ptr<isotp::Protocol> isotp_;
  FlashablePtr flash_;
  uds::Authenticator auth_;
  std::string key_;

  size_t left_{}, sent_{};

  void onFail(const std::string &error);
  void request(gsl::span<uint8_t> data, uint8_t expected, isotp::Protocol::RecvPacketCallback &&cb);
  void sendLoad();
  void do_erase();
  void do_erase_recv();
  void do_request_download();
  void do_download_recv();

  bool checkResponse(uint8_t res, uint8_t expected);
  bool checkNegative(uint8_t res, isotp::Packet &packet);
};

MazdaT1Flasher::MazdaT1Flasher(Flasher::Callbacks *callbacks,
                               const std::string &key, std::shared_ptr<isotp::Protocol> isotp)
: Flasher(callbacks), key_(key), isotp_(std::move(isotp)), auth_(std::bind(&MazdaT1Flasher::onAuthenticated, this, std::placeholders::_1, std::placeholders::_2)) {

}

void MazdaT1Flasher::flash(FlashablePtr flashable) {
  flash_ = flashable;
  auth_.auth(key_, uds::Protocol::create(isotp_), 0x85);
}

void MazdaT1Flasher::onFail(const std::string &error) {
  callbacks_->onError(error);
}

bool MazdaT1Flasher::checkResponse(uint8_t res, uint8_t expected) {
  if (expected != res) {
    std::stringstream ss;
    ss << "Unexpected response id: 0x" << std::hex << res << ". Expected 0x" << expected;
    onFail(ss.str());
    return false;
  }
  return true;
}

bool MazdaT1Flasher::checkNegative(uint8_t res, isotp::Packet &packet) {
  if (res == UDS_RES_NEGATIVE) {
    std::stringstream ss;
    ss << "received negative UDS response";
    if (!packet.eof()) {
      ss << ": 0x" << std::hex << static_cast<int>(packet.next());
    }
    onFail(ss.str());
    return false;
  }
  return true;
}

void MazdaT1Flasher::request(gsl::span<uint8_t> data, uint8_t expected, isotp::Protocol::RecvPacketCallback &&cb) {
  isotp_->request(isotp::Packet(data), [this, expected, cb(std::move(cb))](isotp::Error error, isotp::Packet &&packet) {
    if (error != isotp::Error::Success) {
      onFail(isotp::strError(error));
      return;
    }
    if (packet.eof()) {
      onFail("packet size was 0");
      return;
    }
    uint8_t id = packet.next();
    if (checkNegative(id, packet) && checkResponse(id, expected)) {
      cb(error, std::move(packet));
    }
  });
}

void MazdaT1Flasher::onAuthenticated(bool success, const std::string &error) {
  if (!success) {
    onFail(error);
    return;
  }
  do_erase();
}

void MazdaT1Flasher::do_erase_recv() {
  isotp_->recvPacketAsync([this](isotp::Error error, isotp::Packet &&packet) {
    if (error != isotp::Error::Success) {
      onFail(isotp::strError(error));
    }

    if (packet.eof()) {
      onFail("packet size was 0");
      return;
    }

    uint8_t id = packet.next();
    if (id == UDS_RES_NEGATIVE) {
      if (packet.size() == 3) {
        if (packet.next() == 0xB1) {
          uint8_t code = packet.next();
          if (code == 0x78) {
            // Still erasing
            do_erase_recv();
            return;
          }
        }
      }
      onFail("Received negative response while erasing");
    }

    if (id == 0xF1) {
      do_request_download();
    } else {
      onFail("received unexpected id while erasing");
    }
  });
}

void MazdaT1Flasher::do_erase() {
  std::array<uint8_t, 4> eraseRequest = {0xB1, 0x00, 0xB2, 0x00};
  // Erasing is weird
  do_erase_recv();
  isotp_->send(isotp::Packet(eraseRequest));
}

void MazdaT1Flasher::do_request_download() {
  // Send address...size
  std::array<uint8_t, 9> msg;
  msg[0] = UDS_REQ_REQUESTDOWNLOAD;
  writeBEInt32(flash_->offset(), msg.begin() + 1);
  writeBEInt32(flash_->size(), msg.begin() + 5);
  // Send download request
  request(msg, UDS_RES_REQUESTDOWNLOAD, [this](isotp::Error error, isotp::Packet &&packet) {
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

  do_download_recv();
  isotp_->send(isotp::Packet(data));
}

void MazdaT1Flasher::do_download_recv() {
  isotp_->recvPacketAsync([this](isotp::Error error, isotp::Packet &&packet) {
    if (error != isotp::Error::Success) {
      onFail(isotp::strError(error));
    }

    if (packet.eof()) {
      onFail("packet size was 0");
      return;
    }

    uint8_t id = packet.next();
    if (id == UDS_RES_NEGATIVE) {
      if (packet.size() == 3) {
        if (packet.next() == UDS_REQ_TRANSFERDATA) {
          uint8_t code = packet.next();
          if (code == 0x78) {
            // Still writing
            do_download_recv();
            return;
          }
        }
      }
      onFail("Received negative response while flashing");
    }

    if (id == UDS_RES_TRANSFERDATA) {
      callbacks_->onProgress(static_cast<double>(sent_) / flash_->size());
      if (left_ == 0) {
        callbacks_->onCompletion();
      } else {
        sendLoad();
      }
    } else {
      onFail("received unexpected id while flashing");
    }
  });
}

FlasherPtr Flasher::createT1(Flasher::Callbacks* callbacks, const std::string& key, std::shared_ptr<isotp::Protocol> isotp) {
  return std::make_shared<MazdaT1Flasher>(callbacks, key, isotp);
}

