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

#ifndef LT_FLASHER_H
#define LT_FLASHER_H

#include <atomic>
#include <memory>
#include <string>

#include "support/asyncroutine.h"
#include "auth/auth.h"
#include "flashmap.h"

class PlatformLink;

namespace lt {
namespace flash {

struct Options {
    auth::Options auth;
};

/**
 * An interface for flashing ROMs
 */
class Flasher : public AsyncRoutine {
  public:
    virtual ~Flasher() = default;

    /* Flash map. Returns false if canceled. */
    virtual bool flash(const FlashMap &flashable) = 0;

    /* Cancels the active flash */
    virtual void cancel() = 0;
};
using FlasherPtr = std::unique_ptr<Flasher>;

} // namespace flash
} // namespace lt

#endif // LT_FLASHER_H
