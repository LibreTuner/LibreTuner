#include "tabledefinitions.h"
#include "definitions.h"
#include "tablelocations.h"

#include <cassert>


static TableAxis RPM_500("RPM", 500, 500);
static TableAxis LOAD_3125("Load", 0.3125, 0.0625);
static TableAxis LOAD_125("Load", 0.125, 0.0625);

static TableDefinition MS6Definitions[] = {
    TableDefinition(0, "Speed Limiter (fuel cut)", "Maximum speed before fuel is cut", TABLE_1D, TCAT_LIMITER, TDATA_FLOAT),
    TableDefinition(1, "Speed Limiter (throttle close)", "Maximum speed before the throttle plate is closed", TABLE_1D, TCAT_LIMITER, TDATA_FLOAT),
    TableDefinition(2, "Some Timing Table", "Unknown", TABLE_2D, TCAT_MISC, TDATA_FLOAT, 8, &RPM_500, 28, &LOAD_3125, -20, 60), // Change the name!
    TableDefinition(3, "Maximum Ignition Timing (Open Loop)", "Maximum ignition timing during open loop operation", TABLE_2D, TCAT_MISC, TDATA_FLOAT, 14, &RPM_500, 31, &LOAD_125, -20, 60)
};





size_t Mazdaspeed6Definitions::count() const
{
    // return 4;
    return sizeof(MS6Definitions) / sizeof(TableDefinition);
}



const TableDefinition * Mazdaspeed6Definitions::definitions() const
{
    return MS6Definitions;
}




static uint32_t L38K_Locations[] = {
    0x836C, // Speed Limiter (fuel cut)
    0x9AE4, // Speed Limiter (throttle close)
    0x1CE84, // Some timing table (TODO: Change name)
    0x1FE54, // Maximumum Ignition Timing (Open Loop)
};



uint32_t MS6_L38K_Locations::get(int tableId)
{
    assert(tableId < sizeof(L38K_Locations));
    return L38K_Locations[tableId];
}
