#ifndef TABLEGROUP_H
#define TABLEGROUP_H

#include "rom.h"
#include "definitions/tabledefinitions.h"

#include <vector>

class Table;
typedef std::shared_ptr<Table> TablePtr;

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
    
    /* Returns a table from a table id. If create is true and
     * a table does not exist, creates a new table
     * from the ROM data. Returns nullptr if create is false
     * and the table does not exist. */
    TablePtr get(size_t idx, bool create = true);
    
    /* Creates a new table from data. Returns (false, error) on error and (true, "") on success. */
    std::pair<bool, std::string> set(size_t idx, const uint8_t *data, size_t size);
    
    /* Applies table modifications to data */
    void apply(uint8_t *data, size_t length);
    
private:
    RomDataPtr base_;
    
    std::vector<TablePtr> tables_;
};
typedef std::shared_ptr<TableGroup> TableGroupPtr;

#endif // TABLEGROUP_H
