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

#include "datalink.h"
#include "definitions/definitionmanager.h"
#include "dtcdescriptions.h"
#include "log.h"
#include "protocols/canlog.h"
#include "ui/mainwindow.h"
#include <QApplication>

#include <memory>

class IsoTpInterface;
class IsoTpTest;

class VehicleLink;
class Tune;
class TuneData;

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

    /* Attempts to open a tune. Shows an error dialog on failure and returns nullptr. */
    static std::shared_ptr<TuneData> openTune(const std::shared_ptr<Tune> &tune);

    /* Open tune flasher */
    void flashTune(const std::shared_ptr<TuneData> &tune);

    /* Returns the default datalink. Queries the user to create one
     * if none exist. May return nullptr */
    DataLinkPtr getDataLink();

    /* Returns a vehicle link queried with the default datalink. Yeah it's
     * confusing. Use this one \/*/
    std::unique_ptr<VehicleLink> getVehicleLink();

    /* Queries for an attached vehicle and returns a vehicle link.
     * The returned link may be nullptr if no datalink is attached. If you're
     * confused, use that one /\ */
    std::unique_ptr<VehicleLink> queryVehicleLink();

    const DtcDescriptions &dtcDescriptions() const { return dtcDescriptions_; }

    /* Returns the log */
    Log &log() { return log_; }

private:
    MainWindow *mainWindow_;
    std::unique_ptr<FlashWindow> flashWindow_;
    CanLog canLog_;
    Log log_;
    DtcDescriptions dtcDescriptions_;

    /* Location of home directory. */
    QString home_;
};

#endif // LIBRETUNE_H
