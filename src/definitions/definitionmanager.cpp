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

#include "definitionmanager.h"
#include "definition.h"
#include "libretuner.h"

#include <QDir>
#include <QFile>
#include <QString>

DefinitionManager *DefinitionManager::get() {
  static DefinitionManager definitionmanager;
  return &definitionmanager;
}

DefinitionManager::DefinitionManager() {}

DefinitionPtr DefinitionManager::getDefinition(const std::string &id) {
  for (auto it = definitions_.begin(); it != definitions_.end(); ++it) {
    if ((*it)->id() == id) {
      return *it;
    }
  }
  return nullptr;
}

bool DefinitionManager::load() {
  LibreTuner::get()->checkHome();

  QString listPath = LibreTuner::get()->home() + "/definitions";

  if (!QFile::exists(listPath)) {
    return true;
  }

  QDir defsDir(listPath);
  defsDir.setFilter(QDir::NoFilter);
  for (QFileInfo &info :
       defsDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::NoSort)) {
    if (info.isDir()) {
      DefinitionPtr def = std::make_shared<Definition>();
      if (!def->load(info.filePath())) {
        lastError_ = def->lastError();
        return false;
      }
      definitions_.push_back(def);
    }
  }

  return true;
}

DefinitionPtr DefinitionManager::fromVin(const std::string &vin) const {
  for (const DefinitionPtr &def : definitions_) {
    if (def->matchVin(vin)) {
      return def;
    }
  }
  return nullptr;
}
