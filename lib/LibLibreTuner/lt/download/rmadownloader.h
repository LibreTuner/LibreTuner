#include "downloader.h"

#include "../network/uds/uds.h"

#include <atomic>

namespace lt {
namespace download {

// Downloads using ReadMemoryByAddress (UDS SID 23)
class RMADownloader : public Downloader {
  public:
    RMADownloader(network::UdsPtr &&uds, Options &&options);

    bool download() override;
    void cancel() override;
    virtual std::pair<const uint8_t *, size_t> data() override;

  private:
    network::UdsPtr uds_;

    auth::Options authOptions_;

    /* Next memory location to be read from */
    size_t downloadOffset_{};
    /* Amount of data left to be transfered */
    size_t downloadSize_{};
    /* Total size to be transfered. Used for progress updates */
    size_t totalSize_;

    std::vector<uint8_t> downloadData_;

    std::atomic<bool> canceled_;

    bool update_progress();
};

} // namespace download
} // namespace lt