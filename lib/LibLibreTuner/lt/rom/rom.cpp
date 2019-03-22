/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rom.h"
#include "tableext.h"

#include "definition/platform.h"

#include <cassert>

namespace lt {
    
std::vector<uint8_t> Rom::getRawTableData(const ModelTable *modTable) const {
    std::size_t offset = modTable->offset;
    const TableDefinition *def = modTable->table;
    
    auto regionBegin = data_.begin() + offset;
    auto regionEnd = regionBegin + def->byteSize();
    
    return std::vector(regionBegin, regionEnd);
}    

std::vector<uint8_t> Rom::getRawTableData(std::size_t id) const {
    const ModelTable *modTable = model_->getTable(id);
    if (modTable == nullptr) {
        throw std::runtime_error("invalid table id " + std::to_string(id));
    }
    
    return getRawTableData(modTable);
}

TablePtr Rom::baseTable(std::size_t tableId) const {
    const ModelTable *modTable = model_->getTable(tableId);
    if (modTable == nullptr) {
        throw std::runtime_error(
            "table does not have matching offset in model definition (" +
            std::to_string(tableId) + ")");
    }
    
    TablePtr table = std::make_unique<Table>();
    initializeTable(*table, *modTable->table);
    
    std::vector<uint8_t> raw = getRawTableData(modTable);
    table->deserialize(raw.begin(), raw.end(), model_->platform.endianness);

    return table;
}

bool Tune::dirty() const noexcept {
	for (const TablePtr& table : tables_) {
		if (table && table->dirty()) {
			return true;
		}
	}
	return false;
}

void Tune::clearDirty() noexcept {
	for (const TablePtr& table : tables_) {
		if (table && table->dirty()) {
			table->clearDirty();
		}
	}
}

Table *Tune::getTable(std::size_t id, bool create) {
    if (id < tables_.size()) {
        if (tables_[id]) {
            return tables_[id].get();
        }
    }

    if (create) {
        TablePtr table = base_->baseTable(id);

		const TableDefinition* def = base_->model()->platform.getTable(id);

		if (!def->axisX.empty()) {
			table->setAxisX(getAxis(def->axisX, true));
		}
		if (!def->axisY.empty()) {
			table->setAxisY(getAxis(def->axisY, true));
		}

        if (id >= tables_.size()) {
            tables_.resize(id + 1);
        }
        tables_[id] = std::move(table);
        return tables_[id].get();
    }
    return nullptr;
}

Table *Tune::setTable(std::size_t id, const uint8_t *data,
                        std::size_t length) {
    TablePtr table = std::make_unique<Table>();

    const TableDefinition *def = base_->model()->platform.getTable(id);
    if (def == nullptr) {
        return nullptr;
    }

    initializeTable(*table, *def);
	table->deserialize(data, data + length, base_->model()->platform.endianness);

    if (!def->axisX.empty()) {
        table->setAxisX(getAxis(def->axisX, true));
    }
    if (!def->axisY.empty()) {
        table->setAxisY(getAxis(def->axisY, true));
    }

	if (id >= tables_.size()) {
		tables_.resize(id + 1);
	}
	tables_[id] = std::move(table);
	return tables_[id].get();
}

struct ADS {
    template <typename T> void operator()() {
        std::vector<T> des(size);
        readBE<T>(data, data + size * sizeof(T), des.begin());
        auto memoryAxis = std::make_shared<lt::MemoryAxis<T>>(name, des.begin(), des.end());

        axis = std::static_pointer_cast<lt::TableAxis>(memoryAxis);
    }

    std::string name;
    const uint8_t *data;
    std::size_t size;
    lt::TableAxisPtr axis;
};

TableAxisPtr Tune::getAxis(const std::string &id, bool create) {
    auto it = axes_.find(id);
    if (it != axes_.end()) {
        return it->second;
    }

    if (!create) {
        return TableAxisPtr();
    }

    const lt::AxisDefinition *def = base_->model()->platform.getAxis(id);
    if (def == nullptr) {
        return TableAxisPtr();
    }

    switch (def->type) {
    case lt::AxisType::Linear: {
        auto axis = std::make_shared<lt::LinearAxis<double>>(
            def->name, def->start, def->increment);
        axes_.emplace(id, axis);
        return axis;
    }
    case lt::AxisType::Memory: {
        std::size_t offset = base_->model()->getAxisOffset(id);
        std::size_t size = def->size;

        if (offset + size * dataTypeSize(def->dataType) > base_->size()) {
            throw std::runtime_error("axis exceeds rom size (rom size: " +
                                     std::to_string(base_->size()) +
                                     ", axis ends at " +
                                     std::to_string(offset + size) + ")");
        }

        ADS ads;
        ads.data = base_->data() + offset;
        ads.size = size;
        ads.name = def->name;

        datatypeToType(def->dataType, ads);
        axes_.emplace(id, ads.axis);
        return ads.axis;
    }
    }

    return TableAxisPtr();
}

} // namespace lt
