#ifndef LT_MAZDAT1
#define LT_MAZDAT1

#include "flasher.h"
#include "network/uds/uds.h"

namespace lt {
namespace flash {

class MazdaT1Flasher : public Flasher {
  public:
    MazdaT1Flasher(network::UdsPtr &&uds_, Options &&options);

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

} // namespace flash
} // namespace lt

#endif