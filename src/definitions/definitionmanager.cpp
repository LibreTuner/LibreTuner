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

#include <toml/toml.hpp>

#include <QDir>
#include <QFile>
#include <QString>

DefinitionManager *DefinitionManager::get() {
    static DefinitionManager definitionmanager;
    return &definitionmanager;
}

DefinitionManager::DefinitionManager() = default;

definition::MainPtr DefinitionManager::find(const std::string &id) {
    for (const definition::MainPtr &def : definitions_) {
        if (def->id == id) {
            return def;
        }
    }
    return nullptr;
}

void DefinitionManager::load() {
    LibreTuner::get()->checkHome();

    QString listPath = LibreTuner::get()->home() + "/definitions";

    if (!QFile::exists(listPath)) {
        return;
    }

    QDir defsDir(listPath);
    defsDir.setFilter(QDir::NoFilter);
    for (QFileInfo &info : defsDir.entryInfoList(
             QDir::NoDotAndDotDot | QDir::Dirs, QDir::NoSort)) {
        if (info.isDir()) {
            definition::MainPtr def = std::make_shared<definition::Main>();
            std::ifstream file(info.filePath().toStdString());
            if (!file.good()) {
                throw std::runtime_error("Failed to open definition file");
            }
            def->load(file);
            definitions_.push_back(def);
        }
    }
}

definition::MainPtr DefinitionManager::fromVin(const std::string &vin) const {
    for (const definition::MainPtr &def : definitions_) {
        if (def->matchVin(vin)) {
            return def;
        }
    }
    return nullptr;
}
