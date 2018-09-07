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

#include "tune.h"
#include "definitions/definition.h"
#include "libretuner.h"
#include "rom.h"
#include "table.h"
#include "tablegroup.h"

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QXmlStreamReader>

#include <iostream>

#include "util.hpp"
#include <cassert>

Tune::Tune(const TuneMeta &tune) : meta_(tune) {
    // Locate the base rom from the id
    rom_ = RomManager::get()->loadId(meta_.baseId);

    tables_ = std::make_shared<TableGroup>(rom_);

    // Load tables
    QFile file(LibreTuner::get()->home() + "/tunes/" +
               QString::fromStdString(meta_.path));
    if (!file.open(QFile::ReadOnly)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    QXmlStreamReader xml(&file);
    if (xml.readNextStartElement()) {
        if (xml.name() == "tables") {
            readTables(xml);
        } else {
            xml.raiseError(QObject::tr("Unexpected element"));
        }
    }

    if (xml.error()) {
        throw std::runtime_error(QString("%1\nLine %2, column %3")
                                     .arg(xml.errorString())
                                     .arg(xml.lineNumber())
                                     .arg(xml.columnNumber())
                                     .toStdString());
    }
}

void Tune::readTables(QXmlStreamReader &xml) {
    assert(xml.isStartElement() && xml.name() == "tables");

    while (xml.readNextStartElement()) {
        if (xml.name() != "table") {
            xml.raiseError("Unexpected element in tables");
            return;
        }

        if (!xml.attributes().hasAttribute("id")) {
            xml.raiseError("Expected id attribute");
            return;
        }

        bool valid;
        int id = xml.attributes().value("id").toInt(&valid);
        if (!valid) {
            xml.raiseError("id is not a number");
            return;
        }

        if (id >= tables_->count()) {
            xml.raiseError("id is out of range");
            return;
        }

        QByteArray data =
            QByteArray::fromBase64(xml.readElementText().toLatin1());

        tables_->set(id, gsl::make_span(reinterpret_cast<const uint8_t *>(data.data()),
                               data.size()));
    }
}

void Tune::save() {
    QFile file(LibreTuner::get()->home() + "/tunes/" +
               QString::fromStdString(meta_.path));
    if (!file.open(QFile::WriteOnly)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-1); // tabs > spaces

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE tune>");
    xml.writeStartElement("tables");

    for (int i = 0; i < tables_->count(); ++i) {
        TablePtr table = tables_->get(i, false);
        if (table && table->modified()) {
            xml.writeStartElement("table");
            xml.writeAttribute("id", QString::number(i));
            std::vector<uint8_t> data;
            data.resize(table->rawSize());
            assert(table->serialize(data));
            xml.writeCharacters(
                QString(QByteArray(reinterpret_cast<const char *>(data.data()),
                                   data.size())
                            .toBase64()));
            xml.writeEndElement();
        }
    }
    xml.writeEndElement();
    xml.writeEndDocument();

    if (xml.hasError()) {
        throw std::runtime_error("unknown XML error while writing tune");
    }
}

void Tune::apply(gsl::span<uint8_t> data) {
    tables_->apply(data);

    // Checksums
    rom_->subDefinition()->checksums()->correct(data);
}
