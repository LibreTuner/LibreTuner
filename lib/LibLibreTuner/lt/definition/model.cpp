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

}