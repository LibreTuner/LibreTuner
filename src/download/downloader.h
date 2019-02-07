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

#ifndef DOWNLOADINTERFACE_H
#define DOWNLOADINTERFACE_H

#include <QFile>
#include <QString>

#include <atomic>
#include <memory>

#include "asyncroutine.h"
#include "udsauthenticator.h"


class CanInterface;

class PlatformLink;

namespace download {
    
struct Options {
    std::string key;
    std::size_t size;
};

class Downloader : public AsyncRoutine {
public:
    virtual ~Downloader() = default;

    /* Starts downloading. Calls updateProgress if possible.
     * Returns false if canceled. */
    virtual bool download() = 0;

    /* Cancels the active download */
    virtual void cancel() = 0;

    /* Returns the downloaded data */
    virtual std::pair<const uint8_t*, size_t> data() = 0;
};
using DownloadInterfacePtr = std::shared_ptr<Downloader>;

// Downloads using ReadMemoryByAddress (UDS SID 23)
class RMADownloader : public Downloader {
public:
    RMADownloader(const PlatformLink &link, const Options &options);

    bool download() override;
    void cancel() override;
    virtual std::pair<const uint8_t*, size_t> data() override;

private:
    auth::UdsAuthenticator auth_;

    std::unique_ptr<uds::Protocol> uds_;

    std::string key_;

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

/* Creates a downloader from an id and platform link.
 * Returns nullptr if id does not exist. */
std::unique_ptr<Downloader> get_downloader(const std::string &id, const PlatformLink &link, const Options &options);

}

#endif // DOWNLOADINTERFACE_H
