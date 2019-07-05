#ifndef LT_MODELDEF_H
#define LT_MODELDEF_H

#include <string>
#include <unordered_map>
#include <vector>

#include "checksum.h"
#include "table.h"

namespace lt
{

struct Platform;

/* Used to identify models */
class Identifier
{
public:
    template <class InputIt>
    Identifier(uint32_t offset, InputIt first, InputIt end)
        : offset_(offset), data_(first, end)
    {
    }

    uint32_t offset() const { return offset_; }

    const uint8_t * data() const { return data_.data(); }

    size_t size() const { return data_.size(); }

private:
    uint32_t offset_;
    std::vector<uint8_t> data_;
};

/* Model definition. Includes the table locations */
struct Model
{
public:
    explicit Model(const Platform & _platform) : platform(_platform) {}

    const Platform & platform;
    std::string id;
    std::string name;
    Checksums checksums;

    /* Tables */
    std::unordered_map<std::string, TableDefinition> tables;

    // TODO: inheritance-based system like tables.
    std::unordered_map<std::string, std::size_t> axisOffsets;

    // Identifiers are unique to each model in a platform.
    std::vector<Identifier> identifiers;

    /* Gets the table definition with id `id`. Returns
     * nullptr if the table does not exist. */
    const TableDefinition * getTable(const std::string & id) const;

    std::size_t getAxisOffset(const std::string & id) const noexcept;

    /* Returns true if the provided data is the correct
     * size and matches all identifiers. */
    bool isModel(const uint8_t * data, std::size_t size) const noexcept;
};
using ModelPtr = std::shared_ptr<Model>;

} // namespace lt

#endif
