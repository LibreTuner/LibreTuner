#include "tablegroup.h"

#include <cassert>

TableGroup::TableGroup(RomDataPtr base) : base_(base)
{
    definitions_ = base_->definitions();
    locations_ = base_->locations();
    tables_.resize(definitions_->count());
}



TablePtr TableGroup::get(size_t idx)
{
    assert(idx < tables_.size());
    
    TablePtr &table = tables_[idx];
    if (!table)
    {
        table = base_->getTable(idx);
    }
   
    return table;
}
