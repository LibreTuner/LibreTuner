#include "tabledefinitions.h"
#include "definitions.h"
#include "tablelocations.h"

#include <cassert>


static TableDefinition MS6Definitions[] = {
    TableDefinition(0, "Speed Limiter (fuel cut)", TABLE_1D, TCAT_LIMITER, TDATA_FLOAT),
    TableDefinition(1, "Speed Limiter (throttle close)", TABLE_1D, TCAT_LIMITER, TDATA_FLOAT),
    TableDefinition(2, "Some Timing Table", TABLE_2D, TCAT_MISC, TDATA_FLOAT, 8, 28), // Change the name!
};





size_t Mazdaspeed6Definitions::count() const
{
    return 3;
}



const TableDefinition * Mazdaspeed6Definitions::definitions() const
{
    return MS6Definitions;
}




static uint32_t L38K_Locations[] = {
    0x836C, // Speed Limiter (fuel cut)
    0x9AE4, // Speed Limiter (throttle close)
    0x1CE84, // Some timing table (TODO: Change name)
};



uint32_t MS6_L38K_Locations::get(int tableId)
{
    assert(tableId < sizeof(L38K_Locations));
    return L38K_Locations[tableId];
}
