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
