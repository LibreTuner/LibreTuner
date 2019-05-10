#ifndef LT_MAZDAT1
#define LT_MAZDAT1

#include "../network/uds/uds.h"
#include "flasher.h"

namespace lt {

class MazdaT1Flasher : public Flasher {
public:
    MazdaT1Flasher(network::UdsPtr &&uds_, FlashOptions &&options);

    bool flash(const FlashMap &flashmap) override;
    void cancel() override;

private:
    network::UdsPtr uds_;

    const FlashMap *flash_;
    std::atomic<bool> canceled_;

    size_t left_{}, sent_{};

    auth::Options authOptions_;

    bool sendLoad();
    bool do_erase();
    bool do_request_download();
};

} // namespace lt

#endif