#include "tabledefinitions.h"

#include "definitions/definitions.h"

#include <cassert>


TableDefinition::TableDefinition(int id, const std::string& name, TableType type, TableCategory category, DataType dataType, uint32_t sizeX, uint32_t sizeY, int min, int max) : id_(id), name_(name), type_(type), category_(category), dataType_(dataType), sizeX_(sizeX), sizeY_(sizeY), max_(max), min_(min)
{
}



TableDefinitionsPtr TableDefinitions::get(RomType vehicle)
{
    switch(vehicle)
    {
        case ROM_MAZDASPEED6:
            return std::make_shared<Mazdaspeed6Definitions>();
        default:
            assert(false && "Invalid vehicle");
            return nullptr;
    }
    
    return nullptr;
}



const TableDefinition *TableDefinitions::at(int id) const
{
    assert(id < count());
    return &definitions()[id];
}
