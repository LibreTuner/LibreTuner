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

class Flasher;
typedef std::shared_ptr<Flasher> FlasherPtr;

class CanInterface;
typedef std::shared_ptr<CanInterface> CanInterfacePtr;

class Flashable;



enum FlashMode
{
    FLASH_NONE = 0,
    FLASH_T1, // Uses a CAN interface. Supported: Mazdaspeed 6
    
};

/**
 * An interface for flashing ROMs
 */
class FlashInterface
{
public:
    
};

class Flasher
{
public:
    class Callbacks
    {
    public:
        virtual void onProgress(double percent) =0;
        
        virtual void onError(const std::string &error) =0;
    };
    
    /* Creates a T1 flash interface */
    static FlasherPtr createT1(CanInterfacePtr can);
    
    /* Flash that shit */
    void flash(const Flashable &flashable);
};

#endif // FLASHER_H
