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

#ifndef FLASHER_H
#define FLASHER_H

#include <atomic>
#include <memory>
#include <string>

#include "asyncroutine.h"
#include "flashable.h"
#include "protocols/udsprotocol.h"
#include "udsauthenticator.h"

class PlatformLink;


namespace flash {

class Flasher;
typedef std::shared_ptr<Flasher> FlasherPtr;

class Flashable;
typedef std::shared_ptr<Flashable> FlashablePtr;


struct Options {
    std::string key;
};


/**
 * An interface for flashing ROMs
 */
class Flasher : public AsyncRoutine {
public:
    virtual ~Flasher() = default;

    /* Flash that shit. Returns false if canceled. */
    virtual bool flash(Flashable &flashable) = 0;

    /* Cancels the active flash */
    virtual void cancel() = 0;
};



class MazdaT1Flasher : public Flasher {
public:
    MazdaT1Flasher(const PlatformLink &platform, const Options &options);

    bool flash(Flashable &flashable) override;
    void cancel() override;

private:
    std::string key_;
    std::unique_ptr<uds::Protocol> uds_;
    Flashable *flash_;
    uds::Authenticator auth_;
    std::atomic<bool> canceled_;

    size_t left_{}, sent_{};

    bool sendLoad();
    bool do_erase();
    bool do_request_download();
};

std::unique_ptr<Flasher> get_flasher(const std::string &id, const PlatformLink &link, const Options &options);

}

#endif // FLASHER_H
