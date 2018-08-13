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

#ifndef LIBRETUNE_H
#define LIBRETUNE_H

#include "definitions/definitionmanager.h"
#include "protocols/canlog.h"
#include "datalink.h"
#include "ui/mainwindow.h"
#include <QApplication>

#include <memory>

class IsoTpInterface;
class IsoTpTest;

class TuneEditor;
class Tune;
typedef std::shared_ptr<Tune> TunePtr;

class VehicleLink;

class FlashWindow;

class LibreTuner : public QApplication {
  Q_OBJECT
public:
  LibreTuner(int &argc, char *argv[]);

  ~LibreTuner() override;

  /* Returns the global LibreTune object */
  static LibreTuner *get();

  /* Returns the CAN log. */
  CanLog *canLog() { return &canLog_; }

  /* Checks if the home directory exists and if it does not,
   * creates it. */
  void checkHome();

  QString home() { return home_; }

  /* Open the tune editor */
  void editTune(const TunePtr &tune);

  /* Open tune flasher */
  void flashTune(const TunePtr &tune);

  /* Returns the default datalink. Queries the user to create one
   * if none exist. May return nullptr */
  DataLinkPtr getDataLink();

  /* Returns a vehicle link queried with the default datalink */
  std::unique_ptr<VehicleLink> getVehicleLink();

  /* Queries for an attached vehicle and returns a vehicle link.
     The returned link may be nullptr if no datalink is attached. */
  std::unique_ptr<VehicleLink> queryVehicleLink();

private:
  std::unique_ptr<MainWindow> mainWindow_;
  std::unique_ptr<TuneEditor> tuneEditor_;
  std::unique_ptr<FlashWindow> flashWindow_;
  CanLog canLog_;

  /* Location of home directory. */
  QString home_;
};

#endif // LIBRETUNE_H
