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

#include "flashable.h"
#include "tune.h"
#include "rom.h"
#include "tablegroup.h"

#include <cassert>

Flashable::Flashable(TuneDataPtr tune)
{
    RomDataPtr rom = tune->romData();
    assert(rom);
    
    data_.assign(rom->data(), rom->data() + rom->size());
    
    // Apply tune on top of ROM
    tune->apply(data_.data(), data_.size());
    valid_ = true;
}
