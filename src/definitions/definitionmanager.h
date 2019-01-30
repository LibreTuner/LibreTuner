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

#ifndef DEFINITIONMANAGER_H
#define DEFINITIONMANAGER_H

#include <memory>
#include <string>
#include <vector>

#include <QString>
#include <QAbstractListModel>

namespace definition {
struct Main;
using MainPtr = std::shared_ptr<Main>;
}

/**
 * @todo write docs
 */
class DefinitionManager : public QAbstractListModel {
public:
    static DefinitionManager *get();

    /* Returns the definition with the specified id. Returns
     * nullptr is no such definition exists. */
    definition::MainPtr find(const std::string &id);

    /* Loads rom list and metadata. Returns true if no errors
     * occurred */
    void load();

    /* Attempts to find a definition that matches the vin.
     * Returns nullptr if no definition exists. */
    definition::MainPtr fromVin(const std::string &vin) const;

    const std::vector<definition::MainPtr> &definitions() const { return definitions_; }
    
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const;

private:
    DefinitionManager();

    std::vector<definition::MainPtr> definitions_;
};

#endif // DEFINITIONMANAGER_H
