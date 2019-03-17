#include "platform.h"

namespace lt {

const TableDefinition *Platform::getTable(std::size_t id) const noexcept {
    if (id >= tables.size()) {
        return nullptr;
    }
    return &tables.at(id);
}

ModelPtr Platform::findModel(const std::string &id) const noexcept {
    for (const ModelPtr &model : models) {
        if (model->id == id) {
            return model;
        }
    }
    return nullptr;
}

const AxisDefinition *Platform::getAxis(const std::string &id) const noexcept {
    auto it = axes.find(id);
	if (it == axes.end()) {
		return nullptr;
	}
	return &it->second;
}

ModelPtr Platform::identify(const uint8_t *data, size_t size) const noexcept {
    for (const ModelPtr &model : models) {
        if (model->isModel(data, size)) {
            return model;
        }
    }
    return ModelPtr();
}

} // namespace lt
