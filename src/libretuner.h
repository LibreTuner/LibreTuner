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

#include <QApplication>
#include "ui/mainwindow.h"
#include "protocols/canlog.h"
#include "definitions/definitionmanager.h"

#include <memory>

class IsoTpInterface;
class IsoTpTest;

class TuneEditor;
class Tune;
typedef std::shared_ptr<Tune> TunePtr;

class LibreTuner : public QApplication
{
    Q_OBJECT
public:
    LibreTuner(int &argc, char *argv[]);
    
    /* Returns the global LibreTune object */
    static LibreTuner *get();
    
    /* Returns the CAN log. */
    CanLog *canLog()
    {
        return &canLog_;
    }

    ~LibreTuner();
    
    /* Checks if the home directory exists and if it does not,
     * creates it. */
    void checkHome();
    
    QString home() {
        return home_;
    }
    
    /* Open the tune editor */
    void editTune(TunePtr tune);
    
    /* Open tune flasher */
    void flashTune(TunePtr tune);

private:
    std::unique_ptr<MainWindow> mainWindow_;
    std::unique_ptr<TuneEditor> tuneEditor_;
    CanLog canLog_;
    
    /* Location of home directory. */
    QString home_;
};

#endif // LIBRETUNE_H
