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

#include <memory>
#include <string>

#include "flashable.h"
#include "protocols/udsprotocol.h"
#include "udsauthenticator.h"
#include "asyncroutine.h"

class Flasher;
typedef std::shared_ptr<Flasher> FlasherPtr;

class CanInterface;
typedef std::shared_ptr<CanInterface> CanInterfacePtr;

class Flashable;
typedef std::shared_ptr<Flashable> FlashablePtr;



/**
 * An interface for flashing ROMs
 */
class Flasher : public AsyncRoutine {
public:
  virtual ~Flasher() = default;

  /* Flash that shit */
  virtual void flash(FlashablePtr flashable) = 0;
};



class MazdaT1Flasher : public Flasher {
public:
  MazdaT1Flasher(std::string key,
                 std::unique_ptr<uds::Protocol> &&uds);

  void flash(FlashablePtr flashable) override;

private:
  std::unique_ptr<uds::Protocol> uds_;
  FlashablePtr flash_;
  uds::Authenticator auth_;
  std::string key_;

  size_t left_{}, sent_{};

  void sendLoad();
  void do_erase();
  void do_request_download();
};

#endif // FLASHER_H
