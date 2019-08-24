#include "model.h"

namespace lt
{

const TableDefinition * Model::getTable(const std::string & tableId) const
{
    if (auto it = tables.find(tableId); it != tables.end())
    {
        return &it->second;
    }
    return nullptr;
}

std::size_t Model::getAxisOffset(const std::string & axisId) const noexcept
{
    auto it = axisOffsets.find(axisId);
    if (it == axisOffsets.end())
    {
        return 0;
    }
    return it->second;
}

bool Model::isModel(const uint8_t * data, std::size_t size) const noexcept
{
    if (identifiers.empty())
    {
        // This model is unidentifiable
        return false;
    }

    for (const Identifier & identifier : identifiers)
    {
        // Verify size
        if (identifier.offset() + identifier.size() > size)
            return false;

        if (!std::equal(identifier.data(),
                        identifier.data() + identifier.size(),
                        data + identifier.offset()))
        {
            return false;
        }
    }
    return true;
}

} // namespace lt
