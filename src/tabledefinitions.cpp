#include "tabledefinitions.h"

#include "definitions/definitions.h"

#include <cassert>


TableDefinition::TableDefinition(int id, const std::string& name, const std::string &description, TableType type, TableCategory category, DataType dataType, uint32_t sizeX, const TableAxis *axisX, uint32_t sizeY, const TableAxis *axisY, int min, int max) : id_(id), name_(name), description_(description), type_(type), category_(category), dataType_(dataType), sizeX_(sizeX), sizeY_(sizeY), max_(max), min_(min), axisX_(axisX), axisY_(axisY)
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



TableAxis::TableAxis(const std::string& name, double start, double increment) : name_(name), start_(start), increment_(increment)
{
}



double TableAxis::label(int idx) const
{
    return start_ + increment_ * idx;
}
