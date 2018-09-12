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

#ifndef TABLEDEFINITIONS_H
#define TABLEDEFINITIONS_H

#include <limits>
#include <memory>
#include <string>

#include <QXmlStreamReader>

#define MAX_TABLEID 1000

class TableAxis;
typedef std::shared_ptr<TableAxis> TableAxisPtr;

class TableAxis {
public:
    /* Attempts to load a table axis from an xml element.
     * Returns nullptr on failure and raises an xml error. */
    static TableAxisPtr load(QXmlStreamReader &xml, int iId);

    virtual ~TableAxis() {}

    /* Returns the label for a given axis position */
    virtual double label(int idx) const = 0;

    std::string label() const { return name_; }

    std::string id() const { return id_; }

    int iId() const { return iId_; }

protected:
    TableAxis(const std::string &name, const std::string &id, int iId)
        : name_(name), id_(id), iId_(iId) {}

    std::string name_;
    std::string id_;
    int iId_;
};

#endif // TABLEDEFINITIONS_H
