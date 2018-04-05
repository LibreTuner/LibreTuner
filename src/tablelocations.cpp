#include "tablelocations.h"
#include "definitions/definitions.h"

#include <cassert>


TableLocationsPtr TableLocations::get(RomType type, RomSubType subtype)
{
    switch(type)
    {
        case ROM_MAZDASPEED6:
            switch(subtype)
            {
                case ROM_SUB_L38K:
                    return std::make_shared<MS6_L38K_Locations>();
                default:
                    break;
            }
        default:
            break;
    }
    
    assert(false && "Invalid subtype or type");
}
