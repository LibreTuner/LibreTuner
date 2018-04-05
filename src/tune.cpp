#include "tune.h"
#include "rom.h"
#include "tablegroup.h"

TuneData::TuneData(TunePtr tune) : tune_(tune)
{
    if (!tune->base())
    {
        lastError_ = "tune does not have a valid base";
        valid_ = false;
        return;
    }
    rom_ = std::make_shared<RomData>(tune->base());
    if (!rom_->valid())
    {
        lastError_ = rom_->lastError();
        valid_ = false;
        return;
    }
    
    tables_ = std::make_shared<TableGroup>(rom_);
    valid_ = true;
}
