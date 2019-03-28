#include "mazdat1.h"

#include "auth/udsauthenticator.h"
#include "support/util.hpp"

#include <array>
#include <cassert>

namespace lt {

MazdaT1Flasher::MazdaT1Flasher(network::UdsPtr &&uds, FlashOptions &&options)
    : uds_(std::move(uds)), authOptions_(std::move(options.auth)) {
    assert(uds_);
}

bool MazdaT1Flasher::flash(const FlashMap &flashmap) {
    canceled_ = false;
    flash_ = &flashmap;

    auth::UdsAuthenticator auth(*uds_, authOptions_);
    // auth_.auth(*uds_, auth::Options{key_, 0x85});
    auth.auth();

    if (canceled_)
        return false;
    return do_erase();
}

void MazdaT1Flasher::cancel() { canceled_ = true; }

bool MazdaT1Flasher::do_erase() {
    std::array<uint8_t, 3> eraseRequest = {0x00, 0xB2, 0x00};
    network::UdsPacket res =
        uds_->request(0xB1, eraseRequest.data(), eraseRequest.size());
    if (canceled_) {
        return false;
    }
    return do_request_download();
}

bool MazdaT1Flasher::do_request_download() {
    // Send address...size
    std::array<uint8_t, 8> msg{};
    writeBE<int32_t>(flash_->offset(), msg.begin(), msg.end());
    writeBE<int32_t>(flash_->data().size(), msg.begin() + 4, msg.end());

    // Send download request
    network::UdsPacket _response =
        uds_->request(network::UDS_REQ_REQUESTDOWNLOAD, msg.data(), msg.size());

    if (canceled_) {
        return false;
    }

    // Start uploading
    sent_ = 0;
    left_ = flash_->data().size();
    return sendLoad();
}

bool MazdaT1Flasher::sendLoad() {
    while (left_ != 0) {
        size_t toSend = std::min<size_t>(left_, 0xFFE);
        std::vector<uint8_t> data;
        data.assign(flash_->data().begin() + sent_,
                    flash_->data().begin() + sent_ + toSend);

        sent_ += toSend;
        left_ -= toSend;

        network::UdsPacket res = uds_->request(network::UDS_REQ_TRANSFERDATA,
                                                 data.data(), data.size());

        notifyProgress(static_cast<double>(sent_) / flash_->data().size());
        if (canceled_) {
            return false;
        }
    }
    return true;
}

} // namespace lt