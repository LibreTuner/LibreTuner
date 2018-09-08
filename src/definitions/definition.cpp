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

#include "definition.h"
#include "piddefinitions.h"
#include "tabledefinitions.h"

#include <QDir>

#include <cassert>

SubDefinition::SubDefinition(Definition *definition)
    : definition_(definition) {}

uint32_t SubDefinition::getTableLocation(int tableId, bool *ok) {
    assert(tableId >= 0);
    if (locations_.size() <= tableId) {
        if (ok != nullptr) {
            *ok = false;
        }
        return 0;
    }

    if (ok != nullptr) {
        *ok = true;
    }
    return locations_[tableId];
}

void SubDefinition::loadTables(QXmlStreamReader &xml) {
    bool ok;

    while (xml.readNextStartElement()) {
        if (xml.name() == "table") {
            QXmlStreamAttributes attributes = xml.attributes();
            if (!attributes.hasAttribute("id")) {
                xml.raiseError("No id attribute is defined in table");
                return;
            }

            int tableId = attributes.value("id").toInt(&ok);
            if (!ok) {
                xml.raiseError("Invalid table id: not an integer");
                return;
            }

            if (tableId > MAX_TABLEID) {
                xml.raiseError("id attribute exceeds maximum value (" +
                               QString::number(MAX_TABLEID) + ")");
                return;
            }

            bool foundOffset = false;
            while (xml.readNextStartElement()) {
                if (xml.name() == "offset") {
                    uint32_t offset = xml.readElementText().toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid offset: not an integer");
                        return;
                    }
                    if (locations_.size() <= tableId) {
                        locations_.resize(tableId + 1);
                    }
                    locations_[tableId] = offset;
                    foundOffset = true;
                } else {
                    xml.raiseError("Unexpected element");
                    return;
                }
            }

            if (!foundOffset) {
                xml.raiseError("No offset element is defined for table");
                return;
            }
        } else {
            xml.raiseError("Unexpected element");
            return;
        }
    }
}

void SubDefinition::loadAxes(QXmlStreamReader &xml) {
    bool ok;

    while (xml.readNextStartElement()) {
        if (xml.name() == "axis") {
            QXmlStreamAttributes attributes = xml.attributes();
            if (!attributes.hasAttribute("id")) {
                xml.raiseError("No id attribute is defined in table");
                return;
            }

            int iId = definition_->axisId(
                attributes.value("id").toString().toStdString());
            if (iId == -1) {
                xml.raiseError(
                    "Invalid table id: not defined in main definition");
                return;
            }

            bool foundOffset = false;
            while (xml.readNextStartElement()) {
                if (xml.name() == "offset") {
                    uint32_t offset = xml.readElementText().toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid offset: not an integer");
                        return;
                    }
                    if (axesOffsets_.size() <= iId) {
                        axesOffsets_.resize(iId + 1);
                    }
                    axesOffsets_[iId] = offset;
                    foundOffset = true;
                } else {
                    xml.raiseError("Unexpected element");
                    return;
                }
            }

            if (!foundOffset) {
                xml.raiseError("No offset element is defined for table");
                return;
            }
        } else {
            xml.raiseError("Unexpected element");
            return;
        }
    }
}

void SubDefinition::loadIdentifiers(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() == "identifier") {
            QXmlStreamAttributes attributes = xml.attributes();
            if (!attributes.hasAttribute("offset")) {
                xml.raiseError("No offset attribute defined for identifier");
                return;
            }

            bool ok;
            uint32_t offset = attributes.value("offset").toUInt(&ok, 16);
            if (!ok) {
                xml.raiseError("Invalid offset: not a number");
                return;
            }

            // Find the data
            std::vector<uint8_t> data;
            while (xml.readNextStartElement()) {
                if (xml.name() == "text") {
                    QByteArray sData = xml.readElementText().toUtf8();
                    data.insert(data.end(), sData.begin(), sData.end());
                } else {
                    xml.raiseError("Unexpected element");
                    return;
                }
            }

            identifiers_.emplace_back(offset, data.data(), data.size());
        } else {
            xml.raiseError("Unexpected element");
            return;
        }
    }
}

void SubDefinition::loadChecksums(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() == "checksum") {
            QXmlStreamAttributes attributes = xml.attributes();
            if (!attributes.hasAttribute("mode")) {
                xml.raiseError("No mode attribute defined in checksum");
                return;
            }
            if (!attributes.hasAttribute("offset")) {
                xml.raiseError("No offset attribute defined in checksum");
                return;
            }
            if (!attributes.hasAttribute("size")) {
                xml.raiseError("No size attribute defined in checksum");
                return;
            }
            if (!attributes.hasAttribute("target")) {
                xml.raiseError("No target attribute defined in checksum");
                return;
            }
            QStringRef sMode = attributes.value("mode");
            bool ok;
            uint32_t offset = attributes.value("offset").toUInt(&ok, 16);
            if (!ok) {
                xml.raiseError("Invalid offset: not a number");
                return;
            }
            uint32_t size = attributes.value("size").toUInt(&ok, 16);
            if (!ok) {
                xml.raiseError("Invalid size: not a number");
                return;
            }
            uint32_t target = attributes.value("target").toUInt(&ok, 16);
            if (!ok) {
                xml.raiseError("Invalid target: not a number");
                return;
            }

            Checksum *checksum;
            if (sMode == "basic") {
                checksum = checksums_.addBasic(offset, size, target);
            } else {
                xml.raiseError("Invalid mode attribute");
                return;
            }

            // Read modifiable regions
            while (xml.readNextStartElement()) {
                if (xml.name() == "modify") {
                    attributes = xml.attributes();
                    if (!attributes.hasAttribute("offset")) {
                        xml.raiseError(
                            "No offset attribute defined for modify element");
                        return;
                    }
                    if (!attributes.hasAttribute("size")) {
                        xml.raiseError(
                            "No size attribute defined for modify element");
                        return;
                    }

                    offset = attributes.value("offset").toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid offset: not a number");
                        return;
                    }
                    size = attributes.value("size").toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid size: not a number");
                        return;
                    }

                    checksum->addModifiable(offset, size);
                }
            }
        } else {
            xml.raiseError("Unexpected element");
        }
    }
}

uint32_t SubDefinition::getAxisLocation(int axisId, bool *ok) {
    assert(axisId >= 0);
    if (axisId >= axesOffsets_.size()) {
        if (ok != nullptr) {
            *ok = false;
        }
        return 0;
    }

    if (ok != nullptr) {
        *ok = true;
    }
    return axesOffsets_[axisId];
}

bool SubDefinition::check(gsl::span<const uint8_t> data) {
    for (Identifier &identifier : identifiers_) {
        if (identifier.offset() + identifier.size() > data.size()) {
            return false;
        }

        if (std::equal(data.begin() + identifier.offset(), data.end(),
                       identifier.data(),
                       identifier.data() + identifier.size()) != 0) {
            return false;
        }
    }
    return true;
}

bool SubDefinition::load(const QString &path) {
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        lastError_ = "Failed to open definition file for reading";
        return false;
    }

    QXmlStreamReader xml(&file);

    if (xml.readNextStartElement()) {
        if (xml.name() != "subdefinition") {
            xml.raiseError("Unexpected element");
        }
    }

    while (xml.readNextStartElement()) {
        if (xml.name() == "id") {
            id_ = xml.readElementText().trimmed().toStdString();
        } else if (xml.name() == "name") {
            name_ = xml.readElementText().trimmed().toStdString();
        } else if (xml.name() == "tables") {
            loadTables(xml);
        } else if (xml.name() == "axes") {
            loadAxes(xml);
        } else if (xml.name() == "checksums") {
            loadChecksums(xml);
        } else {
            xml.raiseError("Unexpected element");
        }
    }

    if (!xml.hasError() && id_.empty()) {
        xml.raiseError("No id element is defined");
    }

    if (xml.hasError()) {
        lastError_ = QObject::tr("%1\nLine %2, column %3")
                         .arg(xml.errorString())
                         .arg(xml.lineNumber())
                         .arg(xml.columnNumber())
                         .toStdString();
        return false;
    }
    return true;
}

int Definition::axisId(const std::string &id) {
    auto it = axes_.find(id);
    if (it == std::end(axes_)) {
        return -1;
    }
    return it->second->iId();
}

SubDefinitionPtr Definition::identifySubtype(gsl::span<const uint8_t> data) {
    for (SubDefinitionPtr &def : subtypes_) {
        if (def->check(data)) {
            return def;
        }
    }
    return nullptr;
}

void Definition::readTables(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() != "table") {
            xml.raiseError("Unknown element");
            break;
        }

        TableDefinition definition(this);
        if (definition.load(xml, this)) {
            tables_.addTable(std::move(definition));
        }
    }
}

void Definition::loadAxes(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() != "axis") {
            xml.raiseError("Unexpected element");
            return;
        }

        TableAxisPtr axis = TableAxis::load(xml, lastAxisId_++);
        if (!axis) {
            return;
        }

        axes_.insert(
            std::make_pair<std::string, TableAxisPtr &>(axis->id(), axis));
    }
}

void Definition::loadVins(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() != "vin") {
            xml.raiseError("Unexpected element");
            return;
        }

        try {
            vins_.emplace_back(xml.readElementText().trimmed().toStdString());
        } catch (const std::regex_error &e) {
            xml.raiseError(QStringLiteral("Could not load vin regex: ") +
                           e.what());
        }
    }
}

void Definition::loadPids(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() != "pid") {
            xml.raiseError("Unexpected element");
            return;
        }

        PidDefinition pid;
        pid.valid = true;
        QXmlStreamAttributes attributes = xml.attributes();
        if (!attributes.hasAttribute("id")) {
            xml.raiseError("Could not load PID: no id attribute");
            return;
        }
        bool ok;
        pid.id = attributes.value("id").toUInt(&ok);
        if (!ok) {
            xml.raiseError(
                "Could not load PID: id attribute is not an integer");
            return;
        }

        while (xml.readNextStartElement()) {
            if (xml.name() == "name") {
                pid.name = xml.readElementText().toStdString();
            } else if (xml.name() == "description") {
                pid.description = xml.readElementText().toStdString();
            } else if (xml.name() == "formula") {
                pid.formula = xml.readElementText().toStdString();
            } else if (xml.name() == "unit") {
                pid.unit = xml.readElementText().toStdString();
            } else if (xml.name() == "code") {
                pid.code = xml.readElementText().toUInt(&ok, 16);
                if (!ok) {
                    xml.raiseError(
                        "Could not load PID: code is not an integer");
                    return;
                }
            }
        }

        pids_.add(std::move(pid));
    }
}

bool Definition::loadMain(const QString &path) {
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        lastError_ = "Failed to open main definition file for reading";
        return false;
    }

    QXmlStreamReader xml(&file);

    bool foundSize = false, foundFlashOffset = false, foundFlashSize = false;

    // Default endianness is big
    endianness_ = Endianness::Big;
    downloadMode_ = DM_NONE;
    flashMode_ = FLASH_NONE;

    if (xml.readNextStartElement()) {
        if (xml.name() != "maindefinition") {
            xml.raiseError("Unexpected element");
        }
    }

    bool ok;
    while (xml.readNextStartElement()) {
        if (xml.name() == "name") {
            name_ = xml.readElementText().trimmed().toStdString();
        } else if (xml.name() == "id") {
            id_ = xml.readElementText().trimmed().toStdString();
        } else if (xml.name() == "romsize") {
            size_ = xml.readElementText().toUInt(&ok);
            if (!ok) {
                xml.raiseError("Invalid romsize: not a number");
            }
            foundSize = true;
        } else if (xml.name() == "endianness") {
            QString sEndian = xml.readElementText().trimmed().toLower();
            if (sEndian == "big") {
                endianness_ = Endianness::Big;
            } else if (sEndian == "little") {
                endianness_ = Endianness::Little;
            } else {
                xml.raiseError(
                    "Invalid endianness. Expected values: 'big' or 'little'");
            }
        } else if (xml.name() == "tables") {
            readTables(xml);
        } else if (xml.name() == "transfer") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "flashmode") {
                    QString sMode = xml.readElementText().toLower();
                    if (sMode == "mazdat1") {
                        flashMode_ = FLASH_T1;
                    } else {
                        xml.raiseError("Invalid flash mode");
                    }
                } else if (xml.name() == "downloadmode") {
                    QString sMode = xml.readElementText().toLower();
                    if (sMode == "mazda23") {
                        downloadMode_ = DM_MAZDA23;
                    } else {
                        xml.raiseError("Invalid download mode");
                    }
                } else if (xml.name() == "key") {
                    key_ = xml.readElementText().toStdString();
                } else if (xml.name() == "serverid") {
                    serverId_ = xml.readElementText().toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid server id: not a number");
                    }
                } else {
                    xml.raiseError("Unexpected element");
                }
            }
        } else if (xml.name() == "axes") {
            loadAxes(xml);
        } else if (xml.name() == "flashregion") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "offset") {
                    flashOffset_ = xml.readElementText().toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid flash offset: not a number");
                    }
                    foundFlashOffset = true;
                } else if (xml.name() == "size") {
                    flashSize_ = xml.readElementText().toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid flash size: not a number");
                    }
                    foundFlashSize = true;
                }
            }
        } else if (xml.name() == "vins") {
            loadVins(xml);
        } else if (xml.name() == "pids") {
            loadPids(xml);
        } else if (xml.name() == "baudrate") {
            baudrate_ = xml.readElementText().toUInt(&ok);
            if (!ok) {
                xml.raiseError("Invalid baudrate: not a number");
            }
        } else if (xml.name() == "logmode") {
            QString sMode = xml.readElementText().toLower();
            if (sMode == "uds") {
                logMode_ = LogMode::Uds;
            } else {
                xml.raiseError("Unknown log mode");
            }
        } else {
            xml.raiseError("Unexpected element");
        }
    }

    if (!xml.hasError()) {
        if (id_.empty()) {
            xml.raiseError("No id element is defined");
        } else if (!foundSize) {
            xml.raiseError("No romsize element is defined");
        } else if (!foundFlashOffset) {
            xml.raiseError("No flash offset defined");
        } else if (!foundFlashSize) {
            xml.raiseError("No flash size defined");
        } else {
            // Ensure that the flash region is in bounds
            if (flashOffset_ + flashSize_ > size_) {
                xml.raiseError("Flash region is out of bounds");
            }
        }
    }

    if (xml.hasError()) {
        lastError_ =
            (QObject::tr("error loading ") + path + ": %1\nLine %2, column %3")
                .arg(xml.errorString())
                .arg(xml.lineNumber())
                .arg(xml.columnNumber())
                .toStdString();
        return false;
    }
    return true;
}

SubDefinitionPtr Definition::findSubtype(const std::string &id) {
    auto it = std::find_if(
        subtypes_.begin(), subtypes_.end(),
        [id](SubDefinitionPtr &def) -> bool { return def->id() == id; });
    if (it == subtypes_.end()) {
        return nullptr;
    }

    return *it;
}

bool Definition::loadSubtype(const QString &path) {
    SubDefinitionPtr sub = std::make_shared<SubDefinition>(this);
    if (!sub->load(path)) {
        lastError_ = std::string("Could not load definition file ") +
                     path.toStdString() + ": " + sub->lastError();
        return false;
    }

    subtypes_.push_back(sub);
    return true;
}

TableAxis *Definition::getAxis(const std::string &id) {
    auto it = axes_.find(id);
    if (it == std::end(axes_)) {
        return nullptr;
    }
    return it->second.get();
}

bool Definition::load(const QString &path) {
    QDir dir(path);

    Definition definition;

    if (QFile::exists(path + "/main.xml")) {
        if (!loadMain(path + "/main.xml")) {
            return false;
        }
    } else {
        lastError_ = std::string("No main.xml file in ") + path.toStdString();
        return false;
    }

    for (QFileInfo &info :
         dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files, QDir::NoSort)) {
        if (info.isFile()) {
            if (info.fileName().toLower() != "main.xml") {
                // Subtype
                if (!loadSubtype(info.filePath())) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool Definition::matchVin(const std::string &vin) {
    return std::any_of(vins_.begin(), vins_.end(), [&vin](const auto &pattern) {
        return std::regex_match(vin, pattern);
    });
}
