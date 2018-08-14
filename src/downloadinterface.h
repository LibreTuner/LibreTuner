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

#include <gsl/span>
#include <memory>
#include <atomic>

#include "asyncroutine.h"
#include "udsauthenticator.h"

enum DownloadMode {
  DM_NONE = 0,
  DM_MAZDA23,
};

class CanInterface;

class DownloadInterface : public AsyncRoutine {
public:
  enum Type {
    TYPE_CAN,
    TYPE_J2534,
  };

  virtual ~DownloadInterface() = default;

  /* Starts downloading. Calls updateProgress if possible.
   * Signals onError if an error occurs. */
  virtual void download() = 0;

  /* Cancels the active download */
  virtual void cancel() =0;

  /* Returns the downloaded data */
  virtual gsl::span<const uint8_t> data() =0;
};
using DownloadInterfacePtr = std::shared_ptr<DownloadInterface>;



class Uds23DownloadInterface : public DownloadInterface {
public:
  Uds23DownloadInterface(std::unique_ptr<uds::Protocol> &&uds,
                         std::string key, uint32_t size);

  void download() override;
  void cancel() override;
  virtual gsl::span<const uint8_t> data() override;

private:
  uds::Authenticator auth_;

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

#endif // DOWNLOADINTERFACE_H
