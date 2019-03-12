#ifndef LT_MODELDEF_H
#define LT_MODELDEF_H

#include <string>
#include <unordered_map>
#include <vector>

#include "checksum.h"
#include "table.h"

namespace lt {

struct Platform;

/* Used to identify models */
class Identifier {
public:
    template <class InputIt>
    Identifier(uint32_t offset, InputIt first, InputIt end)
        : offset_(offset), data_(first, end) {}

    uint32_t offset() const { return offset_; }

    const uint8_t *data() const { return data_.data(); }

    size_t size() const { return data_.size(); }

private:
    uint32_t offset_;
    std::vector<uint8_t> data_;
};

struct ModelTable {
	const TableDefinition &table;
	std::size_t offset;
};

/* Model definition. Includes the table locations */
struct Model {
public:
    explicit Model(const Platform &platform);

    const Platform &platform;
    std::string id;
    std::string name;
    Checksums checksums;

    /* Table offsets */
    std::vector<ModelTable> tables;

    std::unordered_map<std::string, std::size_t> axisOffsets;
    std::vector<Identifier> identifiers;

    const ModelTable &getTable(std::size_t index) const;

    std::size_t getAxisOffset(const std::string &id);
};
using ModelPtr = std::shared_ptr<Model>;

} // namespace lt

#endif