#ifndef TABLEDEFINITIONS_H
#define TABLEDEFINITIONS_H

#include <memory>
#include <string>
#include <limits>

#include "rom.h"

class TableDefinitions;
typedef std::shared_ptr<TableDefinitions> TableDefinitionsPtr;

enum TableType
{
    TABLE_1D,
    TABLE_2D,
    TABLE_3D, // Do ECUs ever use these?
};

enum TableCategory
{
    TCAT_LIMITER, // Any type of limiter (RPM limiter, speed limiter)
    TCAT_MISC,
};

enum DataType
{
    TDATA_UINT8,
    TDATA_UINT16,
    TDATA_UINT32,
    TDATA_FLOAT,
    TDATA_INT8,
    TDATA_INT16,
    TDATA_INT32,
};

class TableDefinition;


/**
 * Handles table definitions for each vehicle.
 */
class TableDefinitions
{
public:
    /* Returns the table definitions for a vehicle */
    static TableDefinitionsPtr get(RomType vehicle);
    
    /* Returns the amount of definitions */
    virtual size_t count() const =0;
    
    /* Returns an array of all table definitions */
    virtual const TableDefinition *definitions() const =0;
    
    /* Looks up a definition by id*/
    const TableDefinition *at(int id) const;
    
    virtual ~TableDefinitions() {};
    
private:
    
};



class TableDefinition
{
public:
    TableDefinition(int id, const std::string &name, TableType type, TableCategory category, DataType dataType, uint32_t sizeX = 1, uint32_t sizeY = 1, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max());
    
    std::string name() const
    {
        return name_;
    }
    
    TableType type() const
    {
        return type_;
    }
    
    TableCategory category() const
    {
        return category_;
    }
    
    uint32_t sizeX() const
    {
        return sizeX_;
    }
    
    uint32_t sizeY() const
    {
        return sizeY_;
    }
    
    int id() const
    {
        return id_;
    }
    
    DataType dataType() const
    {
        return dataType_;
    }
    
    int max() const
    {
        return max_;
    }
    
    void setMax(int max)
    {
        max_ = max;
    }
    
    int min() const
    {
        return min_;
    }
    
    void setMin(int min)
    {
        min_ = min;
    }
private:
    int id_;
    std::string name_;
    TableType type_;
    TableCategory category_;
    DataType dataType_;
    uint32_t sizeX_;
    uint32_t sizeY_;
    int max_;
    int min_;
    
    TableDefinition(TableDefinitions &) = delete;
};

#endif // TABLEDEFINITIONS_H
