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

#include "datalink/datalink.h"
#include "datalink/linkdatabase.h"
#include "definitions/definitionmanager.h"
#include "dtcdescriptions.h"
#include "log.h"
#include "protocols/canlog.h"
#include "ui/mainwindow.h"


#include <QApplication>
#include <QAbstractItemModel>

#include <memory>



#define LT() LibreTuner::get()

class PlatformLink;
class Tune;
class TuneData;


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

    QString home() const { return home_; }
    
    /* Returns a path to the data directory */
    std::string dataDirectory() const { return home_.toStdString(); }

    /* Attempts to open a tune. Shows an error dialog on failure and returns nullptr. */
    static std::shared_ptr<TuneData> openTune(const std::shared_ptr<Tune> &tune);

    /* Open tune flasher */
    void flashTune(const std::shared_ptr<TuneData> &tune);

    const DtcDescriptions &dtcDescriptions() const { return dtcDescriptions_; }

    /* Autodetects PassThru interfaces and loads saved datalinks */
    void load_datalinks();
    
    const datalink::LinkDatabase &links() const { return links_; }
    datalink::LinkDatabase &links() { return links_; }

    /* Returns the log */
    Log &log() { return log_; }

    /* Runs the setup dialog */
    void setup();

    /* Returns the current platform or nullptr if one is not selected */
    const definition::MainPtr &platform() const { return currentDefinition_; }
    void setPlatform(const definition::MainPtr &platform);

    /* Returns the selected datalink or nullptr if not is not selected */
    datalink::Link *datalink() const { return currentDatalink_; }
    void setDatalink(datalink::Link *link);

    /* Creates a platform link from the selected platform & datalink. Returns nullptr if
     * one could not be created. */
    std::unique_ptr<PlatformLink> platform_link();

    void saveLinks();

private:
    MainWindow *mainWindow_;
    CanLog canLog_;
    Log log_;
    DtcDescriptions dtcDescriptions_;

    definition::MainPtr currentDefinition_;
    datalink::Link *currentDatalink_{nullptr};

    datalink::LinkDatabase links_;

    /* Location of home directory. */
    QString home_;
};

#endif // LIBRETUNE_H
