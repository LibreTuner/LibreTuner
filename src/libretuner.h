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

#ifndef LIBRETUNER_H
#define LIBRETUNER_H

#include "dtcdescriptions.h"
#include "log.h"
#include "ui/mainwindow.h"


#include <QApplication>
#include <QAbstractItemModel>

#include <memory>


// intolib rewrite stuff
#include "database/definitions.h"
#include "database/roms.h"
#include "database/links.h"

#include "lt/link/platformlink.h"

#include <filesystem>


#define LT() LibreTuner::get()

class Tune;
class TuneData;


class LibreTuner : public QApplication {
    Q_OBJECT

	// intolib rewrite stuff
public:
    LibreTuner(int &argc, char *argv[]);

    ~LibreTuner() override;

    /* Returns the global LibreTune object */
    static LibreTuner *get();

	inline const std::filesystem::path &rootPath() const noexcept { return rootPath_; }

	inline const Definitions &definitions() const noexcept { return definitions_; }
	inline Definitions &definitions() noexcept { return definitions_; }

    inline const Roms &roms() const noexcept { return roms_; }
    inline Roms &roms() noexcept { return roms_; }

	inline const Links &links() const noexcept { return links_; }
	inline Links &links() noexcept { return links_; }

    /* Returns the current platform or nullptr if one is not selected */
    const lt::PlatformPtr &platform() const { return currentPlatform_; }
    void setPlatform(const lt::PlatformPtr &platform);

    /* Returns the selected datalink or nullptr if not is not selected */
    lt::DataLink *datalink() const { return currentDatalink_; }
    void setDatalink(lt::DataLink *link);

	lt::PlatformLink platformLink() const;

    lt::TunePtr openTune(const std::filesystem::path &path) const;
    void saveTune(const lt::Tune &tune, const std::filesystem::path &path) const;

private:
	Definitions definitions_;
	std::filesystem::path rootPath_;

    Roms roms_;
	Links links_;

    lt::DataLink *currentDatalink_{nullptr};
    lt::PlatformPtr currentPlatform_;

	// Legacy stuff
public:
    /* Returns the CAN log. */
    //CanLog *canLog() { return &canLog_; }

    /* Checks if the home directory exists and if it does not,
     * creates it. */
    //void checkHome();

    //QString home() const { return home_; }
    
    /* Returns a path to the data directory */
    //std::string dataDirectory() const { return home_.toStdString(); }

    /* Attempts to open a tune. Shows an error dialog on failure and returns nullptr. */
    static std::shared_ptr<TuneData> openTune(const std::shared_ptr<Tune> &tune);

    /* Open tune flasher */
    void flashTune(const std::shared_ptr<TuneData> &tune);

    inline const DtcDescriptions &dtcDescriptions() const noexcept { return dtcDescriptions_; }

    /* Autodetects PassThru interfaces and loads saved datalinks */
    void load_datalinks();

    /* Returns the log */
    Log &log() { return log_; }

    /* Runs the setup dialog */
    void setup();

    void saveLinks();

private:
    MainWindow *mainWindow_;
    Log log_;
    DtcDescriptions dtcDescriptions_;

    /* Location of home directory. */
    QString home_;
};

#endif // LIBRETUNER_H
