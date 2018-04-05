#ifndef TABLELOCATIONS_H
#define TABLELOCATIONS_H

#include <memory>

#include "rom.h"


class TableLocations;
typedef std::shared_ptr<TableLocations> TableLocationsPtr;

/**
 * Handles table locations from different firmware versions
 */
class TableLocations
{
public:
    /* Returns the table locations for the vehicle and firmware version */
    static TableLocationsPtr get(RomType type, RomSubType subtype);
    
    /* Returns the location for the table id */
    virtual uint32_t get(int tableId) =0;
    
    virtual ~TableLocations() {};
};

#endif // TABLELOCATIONS_H
