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

#include "rommanager.h"
#include "datalink.h"

enum DownloadMode {
  DM_NONE = 0,
  DM_MAZDA23,
};

class CanInterface;

class DownloadInterface {
public:
  enum Type {
    TYPE_CAN,
    TYPE_J2534,
  };

  virtual ~DownloadInterface() = default;

  /* Starts downloading. Calls updateProgress if possible.
   * Signals onError if an error occurs. */
  virtual void download() = 0;

  class Callbacks {
  public:
    /* Signals an update in progress. progress is a value between 0 and 100 */
    virtual void updateProgress(float progress) = 0;

    virtual void downloadError(const QString &error) = 0;

    /* Called when the ROM has finished downloading. */
    virtual void onCompletion(gsl::span<const uint8_t> data) = 0;
  };

  static std::shared_ptr<DownloadInterface> create(Callbacks *callbacks, DataLinkPtr datalink, DefinitionPtr definition);

protected:
  Callbacks *callbacks_;

  explicit DownloadInterface(Callbacks *callbacks);
};

#endif // DOWNLOADINTERFACE_H
