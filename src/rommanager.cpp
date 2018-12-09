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

#include "rommanager.h"
#include "definitions/definition.h"
#include "libretuner.h"

#include <QFileInfo>
#include <cassert>

#include <fstream>
#include <iostream>

RomStore *RomStore::get() {
    static RomStore romManager;
    return &romManager;
}



void RomStore::load() {
    LibreTuner::get()->checkHome();

    QString listPath = LibreTuner::get()->home() + "/" + "roms.xml";

    if (!QFile::exists(listPath)) {
        return;
    }

    QFile listFile(listPath);
    if (!listFile.open(QFile::ReadOnly)) {
        throw std::runtime_error(listFile.errorString().toStdString());
    }

    QXmlStreamReader xml(&listFile);
    // set nextId_ to -1 for checking the highest ROM id
    nextId_ = -1;

    if (xml.readNextStartElement()) {
        if (xml.name() == "roms") {
            readRoms(xml);
        } else {
            xml.raiseError(QObject::tr("Unexpected element"));
        }
    }

    nextId_++;

    if (xml.error()) {
        throw std::runtime_error(QObject::tr("%1\nLine %2, column %3")
                                     .arg(xml.errorString())
                                     .arg(xml.lineNumber())
                                     .arg(xml.columnNumber())
                                     .toStdString());
    }

    emit updateRoms();
}



void RomStore::readRoms(QXmlStreamReader &xml) {
    assert(xml.isStartElement() && xml.name() == "roms");
    roms_.clear();

    while (xml.readNextStartElement()) {
        if (xml.name() != "rom") {
            xml.raiseError("Unexpected element in roms");
            return;
        }

        std::unique_ptr<Rom> rom = std::make_unique<Rom>();
        bool foundId = false;

        std::string modelId;

        // Read ROM data
        while (xml.readNextStartElement()) {
            if (xml.name() == "name") {
                rom->setName(xml.readElementText().trimmed().toStdString());
            } else if (xml.name() == "path") {
                rom->setPath(LibreTuner::get()->home().toStdString() + "/roms/" + xml.readElementText().trimmed().toStdString());
            } else if (xml.name() == "type") {
                QString type = xml.readElementText().toLower();
                definition::MainPtr def =
                    DefinitionManager::get()->find(type.toStdString());
                if (!def) {
                    xml.raiseError("Invalid ROM type (no definition matches '" + type + "')");
                    break;
                }

                rom->setPlatform(std::move(def));
            } else if (xml.name() == "subtype") {
                QString type = xml.readElementText().toLower();
                // Find model
                modelId = type.toStdString();
                // TODO: check if this subtype exists
            } else if (xml.name() == "id") {
                bool ok;
                rom->setId(xml.readElementText().toULong(&ok));
                if (!ok) {
                    xml.raiseError("id is not a valid decimal number");
                }
                foundId = true;
                if (rom->id() > nextId_) {
                    nextId_ = rom->id();
                }
            }
        }

        // Verifications
        if (!xml.hasError()) {
            if (rom->name().empty()) {
                xml.raiseError("ROM name is empty");
                return;
            }
            if (rom->path().empty()) {
                xml.raiseError("ROM path is empty");
                return;
            }
            if (!rom->platform()) {
                xml.raiseError("ROM platform is empty");
                return;
            }
            if (modelId.empty()) {
                xml.raiseError("ROM model is empty");
                return;
            } else {
                // Search for the model
                const definition::ModelPtr &model = rom->platform()->findModel(modelId);
                if (!model) {
                    xml.raiseError(QString::fromStdString("No model found with id '" + modelId + "'"));
                    return;
                }
                rom->setModel(model);
            }
            if (!foundId) {
                xml.raiseError("ROM id is empty or negative");
                return;
            }
        } else {
            return;
        }

        roms_.emplace_back(std::move(rom));
    }
}



void RomStore::save() {
    LibreTuner::get()->checkHome();

    QString listPath = LibreTuner::get()->home() + "/" + "roms.xml";

    QFile listFile(listPath);
    if (!listFile.open(QFile::WriteOnly)) {
        throw std::runtime_error(listFile.errorString().toStdString());
    }

    QXmlStreamWriter xml(&listFile);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-1); // tabs > spaces

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE roms>");
    xml.writeStartElement("roms");
    for (const auto &rom : roms_) {
        xml.writeStartElement("rom");
        xml.writeTextElement("name", QString::fromStdString(rom->name()));
        xml.writeTextElement("path", QString::fromStdString(rom->path()));
        xml.writeTextElement("id", QString::number(rom->id()));
        xml.writeTextElement("type", QString::fromStdString(rom->platform()->id));
        xml.writeTextElement("subtype",
                             QString::fromStdString(rom->model()->id));
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndDocument();
}




void RomStore::addRom(const std::string &name,
                        const definition::MainPtr &definition,
                        const uint8_t *data, size_t size) {
    LibreTuner::get()->checkHome();

    QString romRoot = LibreTuner::get()->home() + "/roms/";
    QString path = QString::fromStdString(name);
    if (QFile::exists(path)) {
        int count = 0;
        do {
            path = QString::fromStdString(name) + QString::number(++count);
        } while (QFile::exists(romRoot + path));
    }

    std::ofstream file((romRoot + path).toStdString(),
                       std::fstream::out | std::fstream::binary);
    file.write(reinterpret_cast<const char *>(data), size);
    file.close();

    // Determine the subtype
    definition::ModelPtr subtype = definition->identify(data, size);
    if (!subtype) {
        throw std::runtime_error(
            "unknown firmware version or this is the wrong vehicle. If "
            "this is the correct vehicle, please submit a bug report so "
            "we can add support for this firmware version.");
    }

    std::unique_ptr<Rom> rom = std::make_unique<Rom>();
    rom->setName(name);
    rom->setPath(path.toStdString());
    rom->setPlatform(definition);
    rom->setModel(subtype);
    rom->setId(nextId_++);
    roms_.emplace_back(std::move(rom));

    emit updateRoms();

    save();
}



Rom *RomStore::fromId(std::size_t id) {
    for (auto &rom : roms_) {
        if (rom->id() == id) {
            return rom.get();
        }
    }

    return nullptr;
}


Rom *RomStore::get(std::size_t index) {
    if (index <= count()) {
        return roms_[index].get();
    }
    return nullptr;
}
    


Tune &RomStore::createTune(Rom &base,
                                  const std::string &name) {
    LibreTuner::get()->checkHome();

    QString tuneRoot = LibreTuner::get()->home() + "/tunes/";
    QString path = QString::fromStdString(name) + ".xml";
    if (QFile::exists(path)) {
        int count = 0;
        do {
            path = QString::fromStdString(name) + QString::number(++count) +
                   ".xml";
        } while (QFile::exists(tuneRoot + path));
    }

    QFile file(tuneRoot + path);
    if (!file.open(QFile::WriteOnly)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    QXmlStreamWriter xml(&file);
    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE tune>");
    xml.writeStartElement("tables");
    xml.writeEndElement();
    xml.writeEndDocument();
    file.close();

    std::unique_ptr<Tune> tune = std::make_unique<Tune>();
    tune->setName(name);
    tune->setPath(path.toStdString());
    tune->setBase(&base);
    tune->setId(nextId_++);
    
    Tune *ptr = tune.get();

    base.addTune(std::move(tune));
    // emit updateTunes();

    saveTunes();

    return *ptr;
}



void RomStore::readTunes(QXmlStreamReader &xml) {
    assert(xml.isStartElement() && xml.name() == "tunes");

    while (xml.readNextStartElement()) {
        if (xml.name() != "tune") {
            xml.raiseError("Unexpected element in tunes");
            return;
        }

        std::unique_ptr<Tune> tune = std::make_unique<Tune>();
        tune->setId(nextId_++);

        // Read ROM data
        while (xml.readNextStartElement()) {
            if (xml.name() == "name") {
                tune->setName(xml.readElementText().toStdString());
            }
            if (xml.name() == "path") {
                tune->setPath(LibreTuner::get()->home().toStdString() + "/tunes/" + xml.readElementText().toStdString());
            }
            if (xml.name() == "base") {
                // Locate the base rom from the ID
                bool ok;
                std::size_t id = xml.readElementText().toULong(&ok);
                if (!ok) {
                    xml.raiseError("Base is not a valid decimal number");
                    return;
                }

                Rom *base = fromId(id);
                if (base == nullptr) {
                    xml.raiseError("Tune does not have a va lid base");
                    return;
                }
                tune->setBase(base);
                base->addTune(std::move(tune));
            }
        }
    }
}



void RomStore::saveTunes() {
    LibreTuner::get()->checkHome();

    QString listPath = LibreTuner::get()->home() + "/" + "tunes.xml";

    QFile listFile(listPath);
    if (!listFile.open(QFile::WriteOnly)) {
        throw std::runtime_error(listFile.errorString().toStdString());
    }

    QXmlStreamWriter xml(&listFile);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-1); // tabs > spaces

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE tunes>");
    xml.writeStartElement("tunes");
    for (const auto &rom : roms_) {
        for (const auto &tune : rom->tunes()) {
            xml.writeStartElement("tune");
            xml.writeTextElement("name", QString::fromStdString(tune->name()));
            xml.writeTextElement("path", QString::fromStdString(tune->path()));
            xml.writeTextElement("base", QString::number(rom->id()));

            xml.writeEndElement();
        }
    }

    xml.writeEndDocument();
}



void RomStore::loadTunes() {
    LibreTuner::get()->checkHome();

    QString listPath = LibreTuner::get()->home() + "/" + "tunes.xml";

    if (!QFile::exists(listPath)) {
        return;
    }

    QFile listFile(listPath);
    if (!listFile.open(QFile::ReadOnly)) {
        throw std::runtime_error(listFile.errorString().toStdString());
    }

    QXmlStreamReader xml(&listFile);

    if (xml.readNextStartElement()) {
        if (xml.name() == "tunes") {
            readTunes(xml);
        } else {
            xml.raiseError(
                QObject::tr("This file is not a tune list document"));
        }
    }

    if (xml.error()) {
        throw std::runtime_error(QObject::tr("%1\nLine %2, column %3")
                                     .arg(xml.errorString())
                                     .arg(xml.lineNumber())
                                     .arg(xml.columnNumber())
                                     .toStdString());
    }

    // emit updateTunes();
}
