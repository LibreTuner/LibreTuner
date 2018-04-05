#include "rom.h"
#include "tabledefinitions.h"
#include "tablelocations.h"
#include "table.h"
#include "libretuner.h"

#include <cassert>

#include <QFile>


void Rom::setType(RomType type)
{
    type_ = type;
    
    switch (type)
    {
        case ROM_NONE:
        case ROM_MAZDASPEED6:
            endianness_ = ENDIAN_BIG;
            break;
            
        default:
            endianness_ = ENDIAN_BIG;
            break;
    }
}


RomData::RomData(RomPtr rom) : rom_(rom)
{
    assert(rom);
    locations_ = TableLocations::get(rom->type(), rom->subType());
    definitions_ = TableDefinitions::get(rom->type());
    
    QFile file(LibreTuner::get()->home() + "/roms/" + QString::fromStdString(rom->path()));
    if (!file.open(QFile::ReadOnly))
    {
        lastError_ = file.errorString().toStdString();
        valid_ = false;
        return;
    }
    
    QByteArray data = file.readAll();
    data_.assign(data.data(), data.data() + data.size());
    
    // TODO: add checksum and check data size
    
    valid_ = true;
}



TablePtr RomData::getTable(int idx)
{
    assert(idx < definitions_->count());
    
    const TableDefinition *def = definitions_->at(idx);
    
    switch(def->type())
    {
        case TABLE_1D:
            switch (def->dataType())
            {
                case TDATA_FLOAT:
                {
                    return std::make_shared<Table1d<float>>(def, rom_->endian(), data_.data() + locations_->get(idx));
                }
            }
        case TABLE_2D:
            switch(def->dataType())
            {
                case TDATA_FLOAT:
                {
                    return std::make_shared<Table2d<float>>(def, rom_->endian(), data_.data() + locations_->get(idx));
                }
            }
    }
    
    assert(false && "unimplemented");
    return nullptr;
}
