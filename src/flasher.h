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
#include "protocols/isotpprotocol.h"

class Flasher;
typedef std::shared_ptr<Flasher> FlasherPtr;

class CanInterface;
typedef std::shared_ptr<CanInterface> CanInterfacePtr;

class Flashable;
typedef std::shared_ptr<Flashable> FlashablePtr;

/**
 * An interface for flashing ROMs
 */
class FlashInterface {
public:
};

class Flasher {
public:
  class Callbacks {
  public:
    virtual void onProgress(double progress) = 0;

    virtual void onCompletion() = 0;

    virtual void onError(const std::string &error) = 0;
  };

  virtual ~Flasher() = default;

  /* Creates a MazdaT1 flash interface */
  static FlasherPtr createT1(Callbacks *callbacks, const std::string &key, std::shared_ptr<isotp::Protocol> isotp);

  /* Flash that shit */
  virtual void flash(FlashablePtr flashable) = 0;

protected:
  explicit Flasher(Callbacks *callbacks);

  Callbacks *callbacks_;
};

#endif // FLASHER_H
