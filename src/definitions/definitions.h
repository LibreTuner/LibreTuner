#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include "tabledefinitions.h"
#include "tablelocations.h"


// Mazdaspeed 6 definitions
class Mazdaspeed6Definitions : public TableDefinitions
{
public:
    size_t count() const override;
    
    const TableDefinition *definitions() const override;
};



class MS6_L38K_Locations : public TableLocations
{
public:
    uint32_t get(int tableId) override;
};

#endif // DEFINITIONS_H
