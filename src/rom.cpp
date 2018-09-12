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
#include "definitions/definition.h"
#include "libretuner.h"
#include "logger.h"
#include "table.h"

#include <cassert>

#include <QFile>

Rom::Rom(const RomMeta &meta) : name_(meta.name) {
    definition::MainPtr main = DefinitionManager::get()->find(meta.modelId);
    if (!main) {
        throw std::runtime_error("definition does not exist");
    }
    definition_ = main->findModel(meta.modelId);
    if (!definition_) {
        throw std::runtime_error("sub-definition '" + meta.modelId +
                                 "' does not exist");
    }

    QFile file(LibreTuner::get()->home() + "/roms/" +
               QString::fromStdString(meta.path));
    if (!file.open(QFile::ReadOnly)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    QByteArray data = file.readAll();
    data_.assign(data.data(), data.data() + data.size());

    // TODO: add checksum and check data size
}



std::unique_ptr<Table> loadTable(Rom& rom, std::size_t tableId)
{
    if (tableId >= rom.definition()->tables.size()) {
        return nullptr;
    }
    
    const definition::Table &tableDef = rom.definition()->main.tables[tableId];
    std::size_t offset = rom.definition()->tables[tableId];
    
    const std::vector<uint8_t> &data = rom.data();
    
    // Verify the size of the rom
    auto begin = data.begin() + offset;
    auto end = begin + tableDef.rawSize();
    
    if (end > data.end()) {
        throw std::runtime_error("end of table exceeds ROM data");
    }
    
    switch (tableDef.type) {
        case TableType::Float:
            return std::make_unique<TableBase<float>>(begin, end, rom.definition()->main.endianness, tableDef.sizeY);
        case TableType::Uint8:
            return std::make_unique<TableBase<uint8_t>>(begin, end, rom.definition()->main.endianness, tableDef.sizeY);
        case TableType::Uint16:
            return std::make_unique<TableBase<uint16_t>>(begin, end, rom.definition()->main.endianness, tableDef.sizeY);
        case TableType::Uint32:
            return std::make_unique<TableBase<uint32_t>>(begin, end, rom.definition()->main.endianness, tableDef.sizeY);
        case TableType::Int8:
            return std::make_unique<TableBase<int8_t>>(begin, end, rom.definition()->main.endianness, tableDef.sizeY);
        case TableType::Int16:
            return std::make_unique<TableBase<int16_t>>(begin, end, rom.definition()->main.endianness, tableDef.sizeY);
        case TableType::Int32:
            return std::make_unique<TableBase<int32_t>>(begin, end, rom.definition()->main.endianness, tableDef.sizeY);
    }
    
    assert(false && "loadTable() unimplemented");
}
