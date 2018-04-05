#ifndef TABLEGROUP_H
#define TABLEGROUP_H

#include "rom.h"
#include "tabledefinitions.h"
#include "table.h"
#include "tablelocations.h"

#include <vector>

/**
 * Handles a group of tables in a tune
 */
class TableGroup
{
public:
    TableGroup(RomDataPtr base);
    
    size_t count() const
    {
        return tables_.size();
    }
    
    TablePtr get(size_t idx);
    
private:
    RomDataPtr base_;
    
    TableDefinitionsPtr definitions_;
    TableLocationsPtr locations_;
    
    std::vector<TablePtr> tables_;
};
typedef std::shared_ptr<TableGroup> TableGroupPtr;

#endif // TABLEGROUP_H
