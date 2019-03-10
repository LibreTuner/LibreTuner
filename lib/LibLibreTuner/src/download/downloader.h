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

#ifndef LT_DOWNLOADER_H
#define LT_DOWNLOADER_H

#include "auth/auth.h"
#include "support/asyncroutine.h"

#include <memory>
#include <vector>

namespace lt {
namespace download {

struct Options {
    auth::Options auth;
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
    virtual std::pair<const uint8_t *, size_t> data() = 0;
};
using DownloaderPtr = std::unique_ptr<Downloader>;

} // namespace download
} // namespace lt

#endif
