#include "model.h"

namespace lt {

const ModelTable* Model::getTable(std::size_t index) const noexcept {
	if (index < tables.size()) {
		return &tables[index];
	}
	return nullptr;
}

std::size_t Model::getAxisOffset(const std::string& id) const noexcept {
	auto it = axisOffsets.find(id);
	if (it == axisOffsets.end()) {
		return 0;
	}
	return it->second;
}

bool Model::isModel(const uint8_t *data, std::size_t size) const noexcept {
    if (identifiers.empty()) {
        return false;
    }
    
    for (const Identifier &identifier : identifiers) {
        if (identifier.offset() + identifier.size() > size) {
            return false;
        }

        if (!std::equal(data + identifier.offset(), data + identifier.offset() + identifier.size(),
                       identifier.data())) {
            return false;
        }
    }
    return true;
}

}
