#ifndef ROM_H
#define ROM_H

#include <string>
#include <memory>
#include <vector>

enum RomType
{
    ROM_NONE = 0,
    ROM_MAZDASPEED6,
};

/* Sub types are used for specific firmware versions */
enum RomSubType
{
    ROM_SUB_NONE = 0,
    
    // Mazdaspeed6 ECU models
    ROM_SUB_L38K, 
};

enum Endianness
{
    ENDIAN_BIG,
    ENGINE_LITTLE,
};

class RomData;
typedef std::shared_ptr<RomData> RomDataPtr;

/* ROM Metadata */
class Rom
{
public:
    void setName(const std::string &name)
    {
        name_ = name;
    }
    
    std::string name() const
    {
        return name_;
    }
    
    void setPath(const std::string &path)
    {
        path_ = path;
    }
    
    std::string path() const
    {
        return path_;
    }
    
    /* Sets the type and calculated endianness */
    void setType(RomType type);
    
    RomType type() const
    {
        return type_;
    }
    
    void setSubType(RomSubType type)
    {
        subType_ = type;
    }
    
    RomSubType subType() const
    {
        return subType_;
    }
    
    int id() const {
        return id_;
    }
    
    void setId(int id)
    {
        id_ = id;
    }
    
    bool bigEndian() const
    {
        return endianness_ == ENDIAN_BIG;
    }
    
    Endianness endian() const
    {
        return endianness_;
    }
    
private:
    std::string name_;
    std::string path_;
    int id_;
    RomType type_ = ROM_NONE;
    RomSubType subType_ = ROM_SUB_NONE;
    Endianness endianness_;
};

typedef std::shared_ptr<Rom> RomPtr;


class TableLocations;
typedef std::shared_ptr<TableLocations> TableLocationsPtr;

class TableDefinitions;
typedef std::shared_ptr<TableDefinitions> TableDefinitionsPtr;

class Table;
typedef std::shared_ptr<Table> TablePtr;

/* The object that actually stores firmware data. */
class RomData
{
public:
    RomData(RomPtr rom);
    
    /* Returns the base table from the table index */
    TablePtr getTable(int idx);
    
    bool valid() const
    {
        return valid_;
    }
    
    std::string lastError() const
    {
        return lastError_;
    }
    
    TableLocationsPtr locations() const
    {
        return locations_;
    }
    
    TableDefinitionsPtr definitions() const
    {
        return definitions_;
    }
    
private:
    RomPtr rom_;
    
    bool valid_;
    std::string lastError_;
    
    TableLocationsPtr locations_;
    TableDefinitionsPtr definitions_;
    
    /* Raw firmware data */
    std::vector<uint8_t> data_;
};

#endif // ROM_H
