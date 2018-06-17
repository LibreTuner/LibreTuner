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
#include "table.h"

#include <cassert>

#include <QFile>

RomData::RomData(RomPtr rom) : rom_(rom) {
  assert(rom);
  definition_ = DefinitionManager::get()->getDefinition(rom->definitionId());
  if (!definition_) {
    lastError_ = "Definition does not exist";
    valid_ = false;
    return;
  }
  subDefinition_ = definition_->findSubtype(rom->subDefinitionId());
  if (!subDefinition_) {
    lastError_ =
        "Sub-definition '" + rom->subDefinitionId() + "' does not exist";
    valid_ = false;
    return;
  }

  QFile file(LibreTuner::get()->home() + "/roms/" +
             QString::fromStdString(rom->path()));
  if (!file.open(QFile::ReadOnly)) {
    lastError_ = file.errorString().toStdString();
    valid_ = false;
    return;
  }

  QByteArray data = file.readAll();
  data_.assign(data.data(), data.data() + data.size());

  // TODO: add checksum and check data size

  valid_ = true;
}

TablePtr RomData::getTable(int idx) {
  const TableDefinition *def = definition_->tables()->at(idx);
  if (!def->valid()) {
    return nullptr;
  }

  // Check if the table location is within the data region
  bool ok;
  uint32_t location = subDefinition_->getTableLocation(idx, &ok);
  if (!ok) {
    return nullptr;
  }

  if (location > data_.size()) {
    // out-of-range
    return nullptr;
  }

  return Table::create(def->type(), def->dataType(), def,
                       definition_->endianness(),
                       gsl::make_span(data_).subspan(location));

  try {
    switch (def->type()) {
    case TABLE_1D:
      switch (def->dataType()) {
      case TDATA_FLOAT: {
        return std::make_shared<Table1d<float>>(
            def, definition_->endianness(),
            gsl::make_span(data_).subspan(location));
      }
      }
    case TABLE_2D:
      switch (def->dataType()) {
      case TDATA_FLOAT: {
        return std::make_shared<Table2d<float>>(
            def, definition_->endianness(),
            gsl::make_span(data_).subspan(location));
      }
      }
    }
  } catch (const std::out_of_range &err) {
    // TODO: log this
    return nullptr;
  }

  assert(false && "unimplemented");
  return nullptr;
}
