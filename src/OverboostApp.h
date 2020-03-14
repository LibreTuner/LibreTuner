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

#include <QAbstractItemModel>
#include <QApplication>

#include <memory>

#include "database/links.h"

#include <definition/platform.h>
#include <filesystem>

class MainWindow;

#define OB() OverboostApp::get()

class OverboostApp : public QApplication
{
    Q_OBJECT

public:
    OverboostApp(int & argc, char * argv[]);

    ~OverboostApp() override;

    /* Returns the global LibreTuner object */
    static OverboostApp * get();

    inline const std::filesystem::path & rootPath() const noexcept { return rootPath_; }

private:
    std::filesystem::path rootPath_;

    // Legacy stuff
public:
    /* Returns the CAN log. */
    // CanLog *canLog() { return &canLog_; }

    /* Checks if the home directory exists and if it does not,
     * creates it. */
    // void checkHome();

    // QString home() const { return home_; }

    /* Returns a path to the data directory */
    // std::string dataDirectory() const { return home_.toStdString(); }

    inline const DtcDescriptions & dtcDescriptions() const noexcept { return dtcDescriptions_; }

    /* Autodetects PassThru interfaces and loads saved datalinks */
    void loadDatalinks();

    // Runs the setup dialog

    void saveLinks();

    const lt::Platforms & platforms() const { return platforms_; }

private:
    DtcDescriptions dtcDescriptions_;

    // Editor window
    MainWindow * mainWindow_;

    // Location of home directory.
    QString home_;

    lt::Platforms platforms_;
};

#endif // LIBRETUNER_H
