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


struct TableDefinition;


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


struct TableAxis
{
public:
    TableAxis(const std::string &name = "", double start = 0, double increment = 1);
    
    /* Returns the label for a given axis position */
    double label(int idx) const;
    
    std::string label() const
    {
        return name_;
    }
    
private:
    std::string name_;
    double start_;
    double increment_;
};


struct TableDefinition
{
public:
    TableDefinition(int id, const std::string &name, const std::string &description, TableType type, TableCategory category, DataType dataType, uint32_t sizeX = 1, const TableAxis *axisX = nullptr, uint32_t sizeY = 1, const TableAxis* axisY = nullptr, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max());
    
    std::string name() const
    {
        return name_;
    }
    
    std::string description() const
    {
        return description_;
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
    
    int min() const
    {
        return min_;
    }
    
    const TableAxis *axisX() const
    {
        return axisX_;
    }
    
    const TableAxis *axisY() const
    {
        return axisY_;
    }

private:
    int id_;
    std::string name_;
    std::string description_;
    TableType type_;
    TableCategory category_;
    DataType dataType_;
    uint32_t sizeX_;
    uint32_t sizeY_;
    int max_;
    int min_;
    const TableAxis *axisX_;
    const TableAxis *axisY_;
    
    TableDefinition(TableDefinitions &) = delete;
};

#endif // TABLEDEFINITIONS_H
